#pragma once

#include <inttypes.h>
#include <avr/interrupt.h>

ISR(TWI_vect);

namespace avr {
namespace i2c {

/** I2C slave interface based on callbacks. */
class Slave {
public:

    static void onDataReceived(void (*)(uint8_t data));
    static void onDataRequested(uint8_t (*)());

    static void init(uint8_t address);
    static void stop();

private:
    static void (*__onDataReceived)(uint8_t);
    static uint8_t (*__onDataRequested)();

    friend void ::TWI_vect();
};

} // namespace i2c
} // namespace avr