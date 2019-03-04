#pragma once

#include <inttypes.h>
#include "avr-utils/Serializable.hpp"

namespace avr {

struct Timestamp;
struct DateTime;



/**
 * Wrapper around a timestamp for type safety and ease of use.
 * Note that the timestamp is from the Y2K epoch, i.e. Midnight Jan 1 2000 UTC.
 */
struct Timestamp {
    uint32_t timestamp;

    Timestamp(uint32_t t = 0)
        : timestamp(t)
    {
    }

    /** Constructs this timestap from the given human-readable DateTime representation. */
    Timestamp(const DateTime& dt);

    /** Expands this timestamp into a human-readable DateTime representation. */
    inline DateTime toDateTime() const;

    // Comparison operators
    bool operator==(const Timestamp& other) const { return timestamp == other.timestamp; }
    bool operator< (const Timestamp& other) const { return timestamp < other.timestamp;  }
    bool operator<=(const Timestamp& other) const { return timestamp <= other.timestamp; }
    bool operator> (const Timestamp& other) const { return timestamp > other.timestamp;  }
    bool operator>=(const Timestamp& other) const { return timestamp >= other.timestamp; }

    // Make this structure serializable
    using Serializer = avr::Serializer<
        avr::Field<&Timestamp::timestamp>
    >;

};



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

    /** Constructs a new DateTime equal to the given timestamp. */
    DateTime(const Timestamp& ts);

    /** Returns the timestamp equivalent to this DateTime. */
    inline Timestamp toTimestamp() const;

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


DateTime Timestamp::toDateTime() const { return DateTime(*this); }
Timestamp DateTime::toTimestamp() const { return Timestamp(*this); }

} // namespace avr