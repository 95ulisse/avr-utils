#pragma once

#include <avr/interrupt.h>
#include <inttypes.h>

ISR(TIMER2_OVF_vect);

namespace avr {

/** Timer2-based counter used to keep track of the milliseconds since the boot. */
class Time {
public:
    
    static void Init();

    static inline uint64_t Millis() {

        // Disable interrupts while reading the volatile value to avoid reading inconsistent data
        uint8_t savedInterrupts = SREG;
        cli();
        uint64_t ms = _ms;
        SREG = savedInterrupts;

        return ms;

    }

private:
    static volatile uint64_t _ms;
    static volatile uint16_t _msFraction;

    friend void TIMER2_OVF_vect();
};

} // namespace avr