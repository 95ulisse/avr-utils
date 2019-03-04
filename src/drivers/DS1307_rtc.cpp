#include <inttypes.h>
#include "avr-utils/drivers/DS1307_rtc.hpp"
#include "avr-utils/i2c_master.hpp"

using I2C = avr::i2c::Master;

static uint8_t bcd2bin(uint8_t x) { return x - 6 * (x >> 4); }
static uint8_t bin2bcd(uint8_t x) { return x + 6 * (x / 10); }

namespace avr {

DateTime RTC::now() const {
    // According to the DS1307 datasheet, first we need to do a write to indicate
    // the first address we want to read from the device (which is 0 in out case
    // since we want to read everything), then we can request all the time info.

    I2C::start(_address, i2c::I2CDirection::Write);
    I2C::write(0);

    DateTime dt;
    I2C::start(_address, i2c::I2CDirection::Read);
    dt.seconds = bcd2bin(I2C::readAck() & 0x7F);
    dt.minutes = bcd2bin(I2C::readAck());
    dt.hours = bcd2bin(I2C::readAck());
    I2C::readAck();
    dt.day = bcd2bin(I2C::readAck());
    dt.month = bcd2bin(I2C::readAck());
    dt.year = bcd2bin(I2C::readNack()) + 2000;
    I2C::stop();

    return dt;
}

void RTC::adjustNow(const DateTime& dt) const {
    I2C::start(_address, i2c::I2CDirection::Write);
    I2C::write(0); // Start the write from the register 0 onwards
    I2C::write(bin2bcd(dt.seconds));
    I2C::write(bin2bcd(dt.minutes));
    I2C::write(bin2bcd(dt.hours));
    I2C::write(bin2bcd(0));
    I2C::write(bin2bcd(dt.day));
    I2C::write(bin2bcd(dt.month));
    I2C::write(bin2bcd(dt.year - 2000));
    I2C::stop();
}

bool RTC::isRunning() const {
    I2C::start(_address, i2c::I2CDirection::Write);
    I2C::write(0);

    I2C::start(_address, i2c::I2CDirection::Read);
    uint8_t val = I2C::readNack();
    I2C::stop();

    // Bit 7 of first register is 1 if the clock is halted, 0 if running
    return (val & 0b10000000) == 0;
}

} // namespace avr