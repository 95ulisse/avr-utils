#pragma once

#include <stddef.h>
#include <inttypes.h>

namespace avr {

enum class ShiftDirection {
    LSBFirst,
    MSBFirst
};

/**
 * Shift out function. Performance optimized of Arduino's default shiftOut.
 * This is only for LSB-First, and uses directly the ports of the chip.
 */
template <typename DataPin, typename ClockPin, ShiftDirection Direction = ShiftDirection::LSBFirst>
inline static void shiftOut(const uint8_t data) {

    // Manually unrolled for loop

    if constexpr (Direction == ShiftDirection::LSBFirst) {

        // bit 0
        if (data & 0b00000001) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 1
        if (data & 0b00000010) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 2
        if (data & 0b00000100) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 3
        if (data & 0b00001000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 4
        if (data & 0b00010000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 5
        if (data & 0b00100000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 6
        if (data & 0b01000000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 7
        if (data & 0b10000000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

    } else {

        // bit 7
        if (data & 0b10000000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 6
        if (data & 0b01000000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 5
        if (data & 0b00100000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 4
        if (data & 0b00010000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 3
        if (data & 0b00001000) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 2
        if (data & 0b00000100) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 1
        if (data & 0b00000010) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

        // bit 0
        if (data & 0b00000001) { DataPin::set(); } else { DataPin::unset(); }
        ClockPin::toggle();
        ClockPin::toggle();

    }

}



/** Maintains the state of a simple 8 bit shift register. */
template <
    typename ClockPin,
    typename LatchPin,
    typename DataPin,
    ShiftDirection Direction = ShiftDirection::LSBFirst
>
class ShiftRegister {
public:

    /** Updates the state of all the outputs. */
    ShiftRegister& set(const uint8_t value) {
        _value = value;
        return *this;
    }

    /** Updates the physical register to reflect the changes. */
    void update() const {

        // Send all the data
        shiftOut<DataPin, ClockPin, Direction>(_value);

        // Toggle latch
        LatchPin::toggle();
        LatchPin::toggle();

    }

private:
    uint8_t _value = 0;
};

} // namespace avr