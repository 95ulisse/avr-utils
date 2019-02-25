#pragma once

#include <inttypes.h>
#include <avr/io.h>
#include <util/twi.h>

#ifndef F_CPU
# error "F_CPU must be defined."
#endif

namespace avr {
namespace i2c {

enum class I2CDirection : uint8_t {
    Write = 0,
    Read = 1
};

class Master {
public:

    static void init() {
        TWBR = (uint8_t)((((F_CPU / 100000UL) / 1) - 16 ) / 2);
    }

    static void stop();

    // Low level operations
    static bool start(uint8_t address, I2CDirection direction);
    static bool start(uint8_t address) { return start(address, I2CDirection::Write); }
    static bool write(uint8_t data);
    static uint8_t readAck();
    static uint8_t readNack();

    // Bulk operations
    static bool transmit(uint8_t address, const uint8_t* data, uint16_t length);
    static bool receive(uint8_t address, uint8_t* data, uint16_t length);

};

} // namespace i2c
} // namespace avr