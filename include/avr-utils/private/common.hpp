#pragma once

#include <inttypes.h>

/** Common enums and template declarations. */

namespace avr {

enum class Port {
    A, B, C, D, E, F
};

enum class PinMode {
    Input,
    InputPullup,
    Output,
    PWM
};

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



template <Port P>               struct port_traits;
template <int I>                struct timer_traits;
template <Port P, uint8_t Mask> struct timer_for_pin;

} // namespace