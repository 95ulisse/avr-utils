#pragma once

#include <inttypes.h>

namespace avr {

struct DateTime {
    unsigned int year;  
    unsigned int month;
    unsigned int day;
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
};

class RTC {
public:

    RTC(uint8_t address)
        : _address(address)
    {
    }

    /** Fills the given DateTime with the current time read from the RTC source. */
    DateTime now();

    /** Updates the current RTC with the given DateTime structure. */
    void adjustNow(const DateTime&);

    /** Returns a value indicating wether the RTC is enabled or not. */
    bool isRunning();

private:
    const uint8_t _address;
};

} // namespace avr;