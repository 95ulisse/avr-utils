#include "avr-utils/i2c_master.hpp"

/**
 * This code is based off I2C-master-lib by g4lvanix:
 * https://github.com/g4lvanix/I2C-master-lib
 */

namespace avr {
namespace i2c {

bool Master::start(uint8_t address, I2CDirection dir) {

    // Take only the 7 least significant bit of the address and combine them with the direction
    address = ((address & 0b01111111) << 1) | (uint8_t) dir;

    // reset TWI control register
    TWCR = 0;
    // transmit START condition 
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    // wait for end of transmission
    while (!(TWCR & (1 << TWINT)));
    
    // check if the start condition was successfully transmitted
    if ((TWSR & 0xF8) != TW_START) {
        return false;
    }
    
    // load slave address into data register
    TWDR = address;
    // start transmission of address
    TWCR = (1 << TWINT) | (1 << TWEN);
    // wait for end of transmission
    while( !(TWCR & (1 << TWINT)) );
    
    // check if the device has acknowledged the READ / WRITE mode
    uint8_t twst = TW_STATUS & 0xF8;
    if ((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK)) {
        return false;
    }
    
    return true;
}

bool Master::write(uint8_t data) {

    // load data into data register
    TWDR = data;
    // start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN);
    // wait for end of transmission
    while (!(TWCR & (1 << TWINT)));
    
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK) {
        return false;
    }
    
    return true;
}

uint8_t Master::readAck() {
    
    // start TWI module and acknowledge data after reception
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); 
    // wait for end of transmission
    while (!(TWCR & (1 << TWINT)));
    // return received data from TWDR
    return TWDR;

}

uint8_t Master::readNack() {
    
    // start receiving without acknowledging reception
    TWCR = (1 << TWINT) | (1 << TWEN);
    // wait for end of transmission
    while (!(TWCR & (1 << TWINT)));
    // return received data from TWDR
    return TWDR;

}

bool Master::transmit(uint8_t address, const uint8_t* data, uint16_t length) {

    if (!start(address, I2CDirection::Write)) {
        return false;
    }
    
    for (uint16_t i = 0; i < length; i++) {
    	if (!write(data[i])) {
            return false;
        }
    }
    
    stop();
    
    return true;
}

bool Master::receive(uint8_t address, uint8_t* data, uint16_t length) {

    if (!start(address, I2CDirection::Read)) {
        return false;
    }
    
    for (uint16_t i = 0; i < (length - 1); i++) {
    	data[i] = readAck();
    }
    data[(length-1)] = readNack();
    
    stop();
    
    return true;
}

void Master::stop() {

    // transmit STOP condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

}


} // namespace i2c
} // namespace avr