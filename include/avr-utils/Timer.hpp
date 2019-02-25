#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "avr-utils/private/common.hpp"
#include "avr-utils/private/device.hpp"

namespace avr {

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