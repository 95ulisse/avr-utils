#pragma once

#include <inttypes.h>
#include "avr-utils/time.hpp"

namespace avr {

class RTC {
public:

    RTC(uint8_t address)
        : _address(address)
    {
    }

    /** Fills the given DateTime with the current time read from the RTC source. */
    DateTime now() const;

    /** Updates the current RTC with the given DateTime structure. */
    void adjustNow(const DateTime&) const;

    /** Returns a value indicating wether the RTC is enabled or not. */
    bool isRunning() const;

private:
    const uint8_t _address;
};

} // namespace avr;