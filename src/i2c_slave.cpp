#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "avr-utils/i2c_slave.hpp"

namespace avr {
namespace i2c {

// Callback pointers
void (*Slave::__onDataReceived)(uint8_t) = nullptr;
uint8_t (*Slave::__onDataRequested)() = nullptr;

void Slave::onDataReceived(void (*f)(uint8_t)) {
    __onDataReceived = f;
}

void Slave::onDataRequested(uint8_t (*f)()) {
    __onDataRequested = f;
}

void Slave::init(uint8_t address) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Load address into TWI address register
        TWAR = address << 1;
        // Set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
        TWCR = (1 << TWIE) | (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
    }

    // Enable interrupts
    sei();
}

void Slave::stop() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Reset the control registers
        TWCR = 0;
        TWAR = 0;
    }
}

} // namespace i2c
} // namespace avr



ISR(TWI_vect)
{
    using namespace avr::i2c;

    switch(TW_STATUS)
    {
        case TW_SR_DATA_ACK:
            // Received data from master, call the receive callback
            if (Slave::__onDataReceived) {
                Slave::__onDataReceived(TWDR);
            }
            TWCR = (1 << TWIE) | (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
            break;
        case TW_ST_SLA_ACK:
        case TW_ST_DATA_ACK:
            // Master is requesting data, call the request callback
            if (Slave::__onDataRequested) {
                TWDR = Slave::__onDataRequested();
            }
            TWCR = (1 << TWIE) | (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
            break;
        case TW_BUS_ERROR:
            // Some sort of erroneous state, prepare TWI to be readdressed
            TWCR = 0;
            TWCR = (1 << TWIE) | (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
            break;
        default:
            TWCR = (1 << TWIE) | (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
            break;
    }
}
