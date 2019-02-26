#pragma once

#include "avr-utils/utility.hpp"

/**
 * This file contains definitions for the following family of AVR microcontrollers:
 * 
 * ATmega48A/PA/88A/PA/168A/PA/328/P
 */

namespace avr {

// Forward declarations
template <int> class Timer;

// Timer traits

#define AVR_UTILS_SPECIALIZE_TIMER_TRAITS(i, modes, prescalers)                           \
    template <> struct timer_traits<i> {                                                  \
        using ValueType = remove_cv_t<remove_reference_t<decltype(TCNT ## i)>>;           \
        static volatile uint8_t& controlRegisterA() { return TCCR ## i ## A; }            \
        static volatile uint8_t& controlRegisterB() { return TCCR ## i ## B; }            \
        static volatile ValueType& counterValueRegister() { return TCNT ## i; }           \
        static volatile ValueType& outputCompareRegisterA() { return OCR ## i ## A; }     \
        static volatile ValueType& outputCompareRegisterB() { return OCR ## i ## B; }     \
        static volatile uint8_t& interruptMaskRegister() { return TIMSK ## i; }           \
                                                                                          \
        /* Some constants to easily access those macros irrespectively of the timer */    \
        static constexpr unsigned int COMA0 = COM  ## i ## A0;                            \
        static constexpr unsigned int COMA1 = COM  ## i ## A1;                            \
        static constexpr unsigned int COMB0 = COM  ## i ## B0;                            \
        static constexpr unsigned int COMB1 = COM  ## i ## B1;                            \
        static constexpr unsigned int TOIE  = TOIE ## i;                                  \
        static constexpr unsigned int OCIEA = OCIE ## i ## A;                             \
        static constexpr unsigned int OCIEB = OCIE ## i ## B;                             \
                                                                                          \
        /* Additional members that contains mappings for modes and prescalers. */         \
        modes                                                                             \
        prescalers                                                                        \
    };

#define AVR_UTILS_NORMAL_MODES(i)                                                         \
    template <TimerMode Mode>                                                             \
    static inline void setMode() {                                                        \
                                                                                          \
        uint8_t reg = controlRegisterA();                                                 \
        if constexpr (Mode == TimerMode::Normal) {                                        \
            reg &= ~( (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1) );                      \
        } else if constexpr (Mode == TimerMode::ClearTimerOnCompareMatch) {               \
            reg &= ~(1 << WGM ## i ## 0);                                                 \
            reg |=  (1 << WGM ## i ## 1);                                                 \
        } else if constexpr (Mode == TimerMode::FastPWM) {                                \
            reg |= (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1);                           \
        } else if constexpr (Mode == TimerMode::PhaseCorrectPWM) {                        \
            reg &= ~(1 << WGM ## i ## 1);                                                 \
            reg |=  (1 << WGM ## i ## 0);                                                 \
        } else {                                                                          \
            static_assert(Mode != Mode, "Unsupported mode for timer " #i "." );           \
        }                                                                                 \
        controlRegisterA() = reg;                                                         \
                                                                                          \
        /* Irrespectively of the mode, this bit is always set to zero */                  \
        controlRegisterB() &= ~(1 << WGM ## i ## 2);                                      \
    }

#define AVR_UTILS_EXTENDED_MODES(i)                                                       \
    template <TimerMode Mode>                                                             \
    static inline void setMode() {                                                        \
        uint8_t regA = controlRegisterA();                                                \
        uint8_t regB = controlRegisterB();                                                \
                                                                                          \
        if constexpr (Mode == TimerMode::Normal) {                                        \
            regA &= ~( (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1) );                     \
            regB &= ~( (1 << WGM ## i ## 2) | (1 << WGM ## i ## 3) );                     \
        } else if constexpr (Mode == TimerMode::ClearTimerOnCompareMatch) {               \
            regA &= ~( (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1) );                     \
            regB &= ~(1 << WGM ## i ## 3);                                                \
            regB |= (1 << WGM ## i ## 2);                                                 \
        } else if constexpr (Mode == TimerMode::FastPWM) {                                \
            regA |= (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1);                          \
            regB |= (1 << WGM ## i ## 2) | (1 << WGM ## i ## 3);                          \
        } else if constexpr (Mode == TimerMode::PhaseCorrectPWM) {                        \
            regA |= (1 << WGM ## i ## 0) | (1 << WGM ## i ## 1);                          \
            regB &= ~(1 << WGM ## i ## 2);                                                \
            regB |= (1 << WGM ## i ## 3);                                                 \
        } else if constexpr (Mode == TimerMode::PhaseFrequencyCorrectPWM) {               \
            regA &= ~(1 << WGM ## i ## 1);                                                \
            regA |= (1 << WGM ## i ## 0);                                                 \
            regB &= ~(1 << WGM ## i ## 2);                                                \
            regB |= (1 << WGM ## i ## 3);                                                 \
        } else {                                                                          \
            static_assert(Mode != Mode, "Unsupported mode for timer " #i "." );           \
        }                                                                                 \
                                                                                          \
        controlRegisterA() = regA;                                                        \
        controlRegisterB() = regB;                                                        \
    }

#define AVR_UTILS_NORMAL_PRESCALERS(i)                                                          \
    template <TimerPrescaler Prescaler>                                                         \
    static inline void setPrescaler() {                                                         \
        uint8_t reg = controlRegisterB();                                                       \
        if constexpr (Prescaler == TimerPrescaler::Off) {                                       \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );        \
        } else if constexpr (Prescaler == TimerPrescaler::NoPrescaler) {                        \
            reg &= ~( (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );                              \
            reg |= (1 << CS ## i ## 0);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By8) {                                \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 2) );                              \
            reg |= (1 << CS ## i ## 1);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By64) {                               \
            reg &= ~(1 << CS ## i ## 2);                                                        \
            reg |= (1 << CS ## i ## 0) | (1 << CS ## i ## 1);                                   \
        } else if constexpr (Prescaler == TimerPrescaler::By256) {                              \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) );                              \
            reg |= (1 << CS ## i ## 2);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By1024) {                             \
            reg &= ~(1 << CS ## i ## 1);                                                        \
            reg |= (1 << CS ## i ## 0) | (1 << CS ## i ## 2);                                   \
        } else {                                                                                \
            static_assert(Prescaler != Prescaler, "Unsupported prescaler for timer " #i ".");   \
        }                                                                                       \
        controlRegisterB() = reg;                                                               \
    }

#define AVR_UTILS_EXTENDED_PRESCALERS(i)                                                        \
    template <TimerPrescaler Prescaler>                                                         \
    static inline void setPrescaler() {                                                         \
        uint8_t reg = controlRegisterB();                                                       \
        if constexpr (Prescaler == TimerPrescaler::Off) {                                       \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );        \
        } else if constexpr (Prescaler == TimerPrescaler::NoPrescaler) {                        \
            reg &= ~( (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );                              \
            reg |= (1 << CS ## i ## 0);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By8) {                                \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 2) );                              \
            reg |= (1 << CS ## i ## 1);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By32) {                               \
            reg &= ~(1 << CS ## i ## 2);                                                        \
            reg |= (1 << CS ## i ## 0) | (1 << CS ## i ## 1);                                   \
        } else if constexpr (Prescaler == TimerPrescaler::By64) {                               \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) );                              \
            reg |= (1 << CS ## i ## 2);                                                         \
        } else if constexpr (Prescaler == TimerPrescaler::By128) {                              \
            reg &= ~(1 << CS ## i ## 1);                                                        \
            reg |= (1 << CS ## i ## 0) | (1 << CS ## i ## 2);                                   \
        } else if constexpr (Prescaler == TimerPrescaler::By256) {                              \
            reg &= ~(1 << CS ## i ## 0);                                                        \
            reg |= (1 << CS ## i ## 1) | (1 << CS ## i ## 2);                                   \
        } else if constexpr (Prescaler == TimerPrescaler::By1024) {                             \
            reg |= (1 << CS ## i ## 0) | (1 << CS ## i ## 1) | (1 << CS ## i ## 2);             \
        } else {                                                                                \
            static_assert(Prescaler != Prescaler, "Unsupported prescaler for timer " #i ".");   \
        }                                                                                       \
        controlRegisterB() = reg;                                                               \
    }

AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    0,
    AVR_UTILS_NORMAL_MODES(0),
    AVR_UTILS_NORMAL_PRESCALERS(0)
)

AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    1,
    AVR_UTILS_EXTENDED_MODES(1),
    AVR_UTILS_NORMAL_PRESCALERS(1)
)

AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    2,
    AVR_UTILS_NORMAL_MODES(2),
    AVR_UTILS_EXTENDED_PRESCALERS(2)
)



// Timer to pin mappings

#define AVR_UTILS_TIMER_FOR_PIN(t, c, port, n)                       \
    template <> struct timer_for_pin<Port::port, 1 << n> {           \
        using timer = Timer<t>;                                      \
        static constexpr TimerChannel channel = TimerChannel::c;     \
    }

AVR_UTILS_TIMER_FOR_PIN(0, A, D, 6);
AVR_UTILS_TIMER_FOR_PIN(0, B, D, 5);
AVR_UTILS_TIMER_FOR_PIN(1, A, B, 1);
AVR_UTILS_TIMER_FOR_PIN(1, B, B, 2);
AVR_UTILS_TIMER_FOR_PIN(2, A, B, 3);
AVR_UTILS_TIMER_FOR_PIN(2, B, D, 3);



// Clean up a bit
#undef AVR_UTILS_SPECIALIZE_TIMER_TRAITS
#undef AVR_UTILS_NORMAL_MODES
#undef AVR_UTILS_NORMAL_PRESCALERS
#undef AVR_UTILS_EXTENDED_MODES
#undef AVR_UTILS_EXTENDED_PRESCALERS
#undef AVR_UTILS_TIMER_FOR_PIN

} // namespace avr