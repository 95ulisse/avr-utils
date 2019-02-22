#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>

namespace avr {

enum class PinPort {
    B, C, D
};

enum class PinMode {
    Input,
    InputPullup,
    Output,
    PWM
};

enum class Timer {
    Timer0A,
    Timer0B,
    Timer1A,
    Timer1B,
    Timer2A,
    Timer2B
};

/**
 * Very simple abstraction over a single IO pin.
 * It basically just wraps the operations with the registers in a more friendly interface.
 */
template <PinPort Port, uint8_t N, PinMode Mode>
struct Pin {

    static constexpr uint8_t Mask = 1 << N;

    // Pin initialization

    static inline void init() {
        if constexpr (Mode == PinMode::Output) {
            *controlRegister() |= Mask;
        } else if constexpr (Mode == PinMode::Input) {
            *controlRegister() &= ~Mask;
        } else if constexpr (Mode == PinMode::InputPullup) {
            *controlRegister() &= ~Mask;
            *outputRegister() |= Mask; // Enable the pullup by default
        } else if constexpr (Mode == PinMode::PWM) {
            
            // We just need Fast PWM Mode with prescaler 64 for all the pwm pins
            *controlRegister() |= Mask;
            switch (PWMTimer()) {
                case Timer::Timer0A:
                case Timer::Timer0B:
                    TCCR0A = (1 << WGM00) | (1 << WGM01);
                    TCCR0B = (1 << CS01) | (1 << CS00);
                    break;
                case Timer::Timer1A:
                case Timer::Timer1B:
                    TCCR1A = (1 << WGM10);
                    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
                    break;
                case Timer::Timer2A:
                case Timer::Timer2B:
                    TCCR2A = (1 << WGM20) | (1 << WGM21);
                    TCCR2B = 1 << CS22;
                    break;
            }

        } else {
            static_assert(Mode != Mode, "Invalid pin mode.");
        }
    }

    // Digital read

    __attribute__((always_inline))
    static inline bool read() {
        static_assert(Mode == PinMode::Input || Mode == PinMode::InputPullup);
        return (*inputRegister() & Mask) != 0;
    }

    // Digital write

    __attribute__((always_inline))
    static inline void set() {
        static_assert(Mode == PinMode::Output);
        *outputRegister() |= Mask;
    }

    __attribute__((always_inline))
    static inline void unset() {
        static_assert(Mode == PinMode::Output);
        *outputRegister() &= ~Mask;
    }

    __attribute__((always_inline))
    static inline void toggle() {
        static_assert(Mode == PinMode::Output);
        *outputRegister() ^= Mask;
    }

    // PWM

    static inline constexpr Timer PWMTimer() {
        // This is written as a series of constexpr ifs to check at compile time
        // that we don't try to make pwm on a pin that does not support it.
        // Also, the `Port != Port` are there to trick the compiler:
        // a `static_assert(false, ...)` always makes the program ill-formed.

        if constexpr (Port == PinPort::B) {
            if constexpr (Mask == 1 << 1) {
                return Timer::Timer1A;          /* Port PB1 maps to OC1A */
            } else if constexpr (Mask == 1 << 2) {
                return Timer::Timer1B;          /* Port PB2 maps to OC1B */
            } else if constexpr (Mask == 1 << 3) {
                return Timer::Timer2A;          /* Port PB3 maps to OC2A */
            } else {
                static_assert(Port != Port, "Pin does not have hardware PWM available.");
            }
        } else if constexpr (Port == PinPort::D) {
            if constexpr (Mask == 1 << 3) {
                return Timer::Timer2B;          /* Port PD3 maps to OC2B */
            } else if constexpr (Mask == 1 << 5) {
                return Timer::Timer0B;          /* Port PD5 maps to OC0B */
            } else if constexpr (Mask == 1 << 6) {
                return Timer::Timer0A;          /* Port PD6 maps to OC0A */
            } else {
                static_assert(Port != Port, "Pin does not have hardware PWM available.");
            }
        } else {
            static_assert(Port != Port, "Pin does not have hardware PWM available.");
        }
    }

    static inline void PWM(const uint8_t value) {
        static_assert(Mode == PinMode::PWM);

        // Since `PWMTimer` is a constexpr functions, all these variables will be constant
        // and the compiler will propagate them.
        volatile uint8_t* valueReg8 = nullptr;
        volatile uint16_t* valueReg16 = nullptr;
        volatile uint8_t* controlReg = nullptr;
        uint8_t mask = 0;
        switch (PWMTimer()) {
            case Timer::Timer0A: valueReg8  = &OCR0A; controlReg = &TCCR0A; mask = 1 << COM0A1; break;
            case Timer::Timer0B: valueReg8  = &OCR0B; controlReg = &TCCR0A; mask = 1 << COM0B1; break;
            case Timer::Timer1A: valueReg16 = &OCR1A; controlReg = &TCCR1A; mask = 1 << COM1A1; break;
            case Timer::Timer1B: valueReg16 = &OCR1B; controlReg = &TCCR1A; mask = 1 << COM1B1; break;
            case Timer::Timer2A: valueReg8  = &OCR2A; controlReg = &TCCR2A; mask = 1 << COM2A1; break;
            case Timer::Timer2B: valueReg8  = &OCR2B; controlReg = &TCCR2A; mask = 1 << COM2B1; break;
        }

        // Handle the special cases of 0% and 100% duty cycles as a common digital write
        if (value == 0 || value == 255) {
            *controlReg &= ~mask; // Disable PWM
            if (value == 255) {
                *outputRegister() |= Mask;
            } else {
                *outputRegister() &= ~Mask;
            }
        } else {
            *controlReg |= mask; // Enable PWM
            if (valueReg8 != nullptr) {
                *valueReg8 = value;
            } else {
                *valueReg16 = value;
            }
        }
    }


    // Architecture-specific port-to-register mapping

    static inline constexpr volatile uint8_t* controlRegister() {
        switch (Port) {
            case PinPort::B: return &DDRB;
            case PinPort::C: return &DDRC;
            case PinPort::D: return &DDRD;
        }
    }

    static inline constexpr volatile uint8_t* outputRegister() {
        switch (Port) {
            case PinPort::B: return &PORTB;
            case PinPort::C: return &PORTC;
            case PinPort::D: return &PORTD;
        }
    }

    static inline constexpr volatile uint8_t* inputRegister() {
        switch (Port) {
            case PinPort::B: return &PINB;
            case PinPort::C: return &PINC;
            case PinPort::D: return &PIND;
        }
    }
};

} // namespace avr