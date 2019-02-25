#pragma once

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <inttypes.h>

#ifndef F_CPU
# error "F_CPU must be defined."
#endif

ISR(TIMER2_OVF_vect);

namespace avr {

/** Timer2-based counter used to keep track of the milliseconds since the boot. */
class Clock {
public:
    
    static void init();

    static inline uint64_t millis() {

        // Disable interrupts while reading the volatile value to avoid reading inconsistent data
        uint64_t ms;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            ms = _ms;
        }

        return ms;

    }

private:
    static volatile uint64_t _ms;
    static volatile uint16_t _msFraction;

    friend void ::TIMER2_OVF_vect();
};

} // namespace avr