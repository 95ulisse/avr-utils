#include "util/Time.hpp"

namespace avr {

#define CLOCKS_PER_MICROSECOND (F_CPU / 1000000UL)

// The timer will tick every 64 cycles, and the overflow will occur every 256 values
#define MICROSECONDS_PER_OVERFLOW ((64 * 256) / CLOCKS_PER_MICROSECOND)

// The whole number of milliseconds to increment every time the overflow handler is called
#define MILLIS_INC (MICROSECONDS_PER_OVERFLOW / 1000)

// The fraction of milliseconds to increment every time the overflow handler is called
#define MILLIS_FRACTION_INC (MICROSECONDS_PER_OVERFLOW % 1000)
#define MILLIS_FRACTION_MAX 1000

volatile uint64_t Time::_ms = 0;
volatile uint16_t Time::_msFraction = 0;

void Time::Init() {

    // Enable Timer2 in Fast PWM mode with prescaler 64
    TCCR2A = (1 << WGM20) | (1 << WGM21);
    TCCR2B = 1 << CS22;

    // Enable overflow interrupt
    TIMSK2 |= (1 << TOIE2);

}

} // namespace avr



ISR(TIMER2_OVF_vect) {

    // Copy the variables on the stack to avoid reading them from memory every time (they are volatile)
    uint64_t ms = Time::_ms;
    uint16_t frac = Time::_msFraction;

    ms += MILLIS_INC;
    frac += MILLIS_FRACTION_INC;
    if (frac >= MILLIS_FRACTION_MAX) {
        ms++;
        frac -= MILLIS_FRACTION_MAX;
    }

    Time::_ms = ms;
    Time::_msFraction = frac;

}