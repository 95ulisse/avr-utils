#include <time.h>

#include "avr-utils/time.hpp"

namespace avr {

// Timestamp -> DateTime conversion
DateTime::DateTime(const Timestamp& ts) {

    struct tm t;
    gmtime_r(&ts.timestamp, &t);

    seconds = t.tm_sec;
    minutes = t.tm_min;
    hours = t.tm_hour;
    day = t.tm_mday;
    month = t.tm_mon;
    year = t.tm_year + 1900;

}

// DateTime -> Timestamp conversion
Timestamp::Timestamp(const DateTime& dt) {

    struct tm t = {};
    t.tm_sec = dt.seconds;
    t.tm_min = dt.minutes;
    t.tm_hour = dt.hours;
    t.tm_mday = dt.day;
    t.tm_mon = dt.month;
    t.tm_year = dt.year - 1900;
    t.tm_wday = -1;
    t.tm_yday = -1;
    t.tm_isdst = -1;

    auto ts = mk_gmtime(&t);
    timestamp = ts == (time_t) -1 ? 0 : ts;

}

} // namespace avr