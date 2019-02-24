#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "Common.hpp"

namespace avr {

enum class TimerMode {
    Normal,
    ClearTimerOnCompareMatch,
    FastPWM,
    PhaseCorrectPWM,
    PhaseFrequencyCorrectPWM
};



enum class TimerPrescaler {
    Off,
    NoPrescaler,
    By8,
    By32,
    By64,
    By128,
    By256,
    By1024
};



enum class TimerChannel {
    A, B
};



template <int I>
struct timer_traits;

// Automatically specialize the timer_traits structure depending on the macros defined by the compiler

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
        uint8_t reg = Traits::controlRegisterB();                                               \
        if constexpr (Prescaler == TimerPrescaler::Off) {                                       \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );        \
        } else if constexpr (Prescaler == TimerPrescaler::NoPrescaling) {                       \
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
        Traits::controlRegisterB() = reg;                                                       \
    }

#define AVR_UTILS_EXTENDED_PRESCALERS(i)                                                        \
    template <TimerPrescaler Prescaler>                                                         \
    static inline void setPrescaler() {                                                         \
        uint8_t reg = Traits::controlRegisterB();                                               \
        if constexpr (Prescaler == TimerPrescaler::Off) {                                       \
            reg &= ~( (1 << CS ## i ## 0) | (1 << CS ## i ## 1) | (1 << CS ## i ## 2) );        \
        } else if constexpr (Prescaler == TimerPrescaler::NoPrescaling) {                       \
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
        Traits::controlRegisterB() = reg;                                                       \
    }

#if defined(TCCR0A) && defined(TCCR0B)
AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    0,
    AVR_UTILS_NORMAL_MODES(0),
    AVR_UTILS_NORMAL_PRESCALERS(0)
)
#endif

#if defined(TCCR1A) && defined(TCCR1B)
AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    1,
    AVR_UTILS_EXTENDED_MODES(1),
    AVR_UTILS_NORMAL_PRESCALERS(1)
)
#endif

#if defined(TCCR2A) && defined(TCCR2B)
AVR_UTILS_SPECIALIZE_TIMER_TRAITS(
    2,
    AVR_UTILS_NORMAL_MODES(2),
    AVR_UTILS_EXTENDED_PRESCALERS(2)
)
#endif



/** Mapping from pins to timer channels. */
template <Port P, uint8_t Mask>
struct timer_for_pin;

#define AVR_UTILS_TIMER_FOR_PIN(timer, c, port, n)          \
    template <> struct timer_for_pin<Port::port, 1 << n> {  \
        using timer = Timer<timer>;                         \
        static constexpr TimerChannel channel = c;          \
    };

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



/** Wrapper around register manipulations for timer configuration. */
template <int I>
class Timer {

    using Traits = timer_traits<I>;
    using ValueType = typename Traits::ValueType;

    // Some timers have values with more than 8 bits,
    // so we need to disable interrupts when reading them to avoid a data race
    static constexpr bool needsLocking = sizeof(ValueType) > 1;

public:
    
    /** Sets the mode of the timer. */
    template <TimerMode Mode>
    static inline void setMode() {
        Traits::template setMode<Mode>();
    }

    /** Sets the prescaler of the timer. */
    template <TimerPrescaler Prescaler>
    static inline void setPrescaler() {
        Traits::template setPrescaler<Prescaler>();
    }

    /** Returns the current value of the counter. */
    static inline ValueType counterValue() {
        if constexpr (needsLocking) {
            ValueType tmp;
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                tmp = Traits::counterValueRegister();
            }
            return tmp;
        } else {
            return Traits::counterValueRegister();
        }
    }

    /** Starts the output for the given channel. */
    template <TimerChannel Channel>
    static inline void startOutput() {
        constexpr auto com0 = Channel == TimerChannel::A ? Traits::COMA0 : Traits::COMB0;
        constexpr auto com1 = Channel == TimerChannel::A ? Traits::COMA1 : Traits::COMB1;

        uint8_t reg = Traits::controlRegisterA();
        reg &= ~(1 << com0);
        reg |=  (1 << com1);
        Traits::controlRegisterA() = reg;
    }

    /** Stops the output for the given channel. */
    template <TimerChannel Channel>
    static inline void stopOutput() {
        constexpr auto com0 = Channel == TimerChannel::A ? Traits::COMA0 : Traits::COMB0;
        constexpr auto com1 = Channel == TimerChannel::A ? Traits::COMA1 : Traits::COMB1;

        Traits::controlRegisterA() &= ~( (1 << com0) | (1 << com1) );
    }

    /** Sets the value of the output compare register. */
    template <TimerChannel Channel>
    static inline void setOutputCompareValue(ValueType x) {
        auto& reg = Channel == TimerChannel::A ? Traits::outputCompareRegisterA() : Traits::outputCompareRegisterB();
        if constexpr (needsLocking) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                reg = x;
            }
        } else {
            reg = x;
        }
    }

    /** Enables the overflow interrupt for this timer. */
    static inline void enableOverflowInterrupt() {
        Traits::interruptMaskRegister() |= (1 << Traits::TOIE);
    }

    /** Disables the overflow interrupt for this timer. */
    static inline void disableOverflowInterrupt() {
        Traits::interruptMaskRegister() &= ~(1 << Traits::TOIE);
    }

    /** Enables the output compare match interrupt for the given channel. */
    template <TimerChannel Channel>
    static inline void enableChannelCompareMatchInterrupt() {
        constexpr auto ocie = Channel == TimerChannel::A ? Traits::OCIEA : Traits::OCIEB;
        Traits::interruptMaskRegister() |= (1 << ocie);
    }

    /** Disables the output compare match interrupt for the given channel. */
    template <TimerChannel Channel>
    static inline void disableChannelCompareMatchInterrupt() {
        constexpr auto ocie = Channel == TimerChannel::A ? Traits::OCIEA : Traits::OCIEB;
        Traits::interruptMaskRegister() &= ~(1 << ocie);
    }

};

} // namespace avr