#include "avr-utils/Clock.hpp"
#include "avr-utils/Timer.hpp"

namespace avr {

#define CLOCKS_PER_MICROSECOND (F_CPU / 1000000UL)

// The timer will tick every 64 cycles, and the overflow will occur every 256 values
#define MICROSECONDS_PER_OVERFLOW ((64 * 256) / CLOCKS_PER_MICROSECOND)

// The whole number of milliseconds to increment every time the overflow handler is called
#define MILLIS_INC (MICROSECONDS_PER_OVERFLOW / 1000)

// The fraction of milliseconds to increment every time the overflow handler is called
#define MILLIS_FRACTION_INC (MICROSECONDS_PER_OVERFLOW % 1000)
#define MILLIS_FRACTION_MAX 1000

volatile uint64_t Clock::_ms = 0;
volatile uint16_t Clock::_msFraction = 0;

void Clock::init() {

    // Enable Timer2 in Fast PWM mode with prescaler 64
    using T = Timer<2>;
    T::setMode<TimerMode::FastPWM>();
    T::setPrescaler<TimerPrescaler::By64>();
    T::enableOverflowInterrupt();

    // Enable interrupts
    sei();

}

} // namespace avr



ISR(TIMER2_OVF_vect) {

    using namespace avr;

    // Copy the variables on the stack to avoid reading them from memory every time (they are volatile)
    uint64_t ms = Clock::_ms;
    uint16_t frac = Clock::_msFraction;

    ms += MILLIS_INC;
    frac += MILLIS_FRACTION_INC;
    if (frac >= MILLIS_FRACTION_MAX) {
        ms++;
        frac -= MILLIS_FRACTION_MAX;
    }

    Clock::_ms = ms;
    Clock::_msFraction = frac;

}
