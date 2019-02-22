#pragma once

#include <stddef.h>
#include <inttypes.h>

namespace avr {

/**
 * Shift out function. Performance optimized of Arduino's default shiftOut.
 * This is only for LSB-First, and uses directly the ports of the chip.
 */
template <typename DataPin, typename ClockPin>
inline static void shiftOut(const uint8_t data) {

    // Manually unrolled for loop

    // bit 0 (LSB)
    if (data & 0B00000001) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 1
    if (data & 0B00000010) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 2
    if (data & 0B00000100) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 3
    if (data & 0B00001000) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 4
    if (data & 0B00010000) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 5
    if (data & 0B00100000) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 6
    if (data & 0B01000000) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

    // bit 7
    if (data & 0B10000000) { DataPin::set(); } else { DataPin::unset(); }
    ClockPin::toggle();
    ClockPin::toggle();

}



/** Maintains the state of a simple 8 bit shift register. */
template <
    typename ClockPin,
    typename LatchPin,
    typename DataPin
>
class ShiftRegister {
public:

    /** Updates the state of all the outputs. */
    ShiftRegister& Set(const uint8_t value) {
        _value = value;
        return *this;
    }

    /** Updates the physical register to reflect the changes. */
    void Update() const {

        // Send all the data
        shiftOut<DataPin, ClockPin>(_value);

        // Toggle latch
        LatchPin::toggle();
        LatchPin::toggle();

    }

private:
    uint8_t _value = 0;
};

} // namespace avr