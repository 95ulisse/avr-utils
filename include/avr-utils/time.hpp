#pragma once

#include <inttypes.h>
#include "avr-utils/Serializable.hpp"

namespace avr {

/** Very basic DateTime structure to represent a time istant. */
struct DateTime {
    unsigned int year;
    unsigned char month;
    unsigned char day;
    unsigned char hours;
    unsigned char minutes;
    unsigned char seconds;

    DateTime(
        unsigned int y = 0, unsigned char mo = 0, unsigned char d = 0,
        unsigned char h = 0 , unsigned char mi = 0, unsigned char s = 0
    )
        : year(y),
          month(mo),
          day(d),
          hours(h),
          minutes(mi),
          seconds(s)
    {
    }

    // Make this structure serializable
    using Serializer = avr::Serializer<
        avr::Field<&DateTime::year>,
        avr::Field<&DateTime::month>,
        avr::Field<&DateTime::day>,
        avr::Field<&DateTime::hours>,
        avr::Field<&DateTime::minutes>,
        avr::Field<&DateTime::seconds>
    >;

};

} // namespace avr