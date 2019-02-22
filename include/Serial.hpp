#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "CircularBuffer.hpp"

#ifndef F_CPU
#error "F_CPU must be defined."
#endif

#ifndef SERIAL_BUFFER_SIZE
#define SERIAL_BUFFER_SIZE 32
#endif

#if defined(UBRR0H)
#define SERIAL_HAVE_SERIAL0
#endif

namespace avr {

enum class SerialConfig : uint8_t {
    Config_5N1 = 0x00,
    Config_6N1 = 0x02,
    Config_7N1 = 0x04,
    Config_8N1 = 0x06,
    Config_5N2 = 0x08,
    Config_6N2 = 0x0A,
    Config_7N2 = 0x0C,
    Config_8N2 = 0x0E,
    Config_5E1 = 0x20,
    Config_6E1 = 0x22,
    Config_7E1 = 0x24,
    Config_8E1 = 0x26,
    Config_5E2 = 0x28,
    Config_6E2 = 0x2A,
    Config_7E2 = 0x2C,
    Config_8E2 = 0x2E,
    Config_5O1 = 0x30,
    Config_6O1 = 0x32,
    Config_7O1 = 0x34,
    Config_8O1 = 0x36,
    Config_5O2 = 0x38,
    Config_6O2 = 0x3A,
    Config_7O2 = 0x3C,
    Config_8O2 = 0x3E
};

/** Buffered UART serial stream. */
class Serial {
public:

    Serial()
        : _readBuffer(SERIAL_BUFFER_SIZE),
          _writeBuffer(SERIAL_BUFFER_SIZE)
    {
    }

    virtual void init(unsigned long baud, SerialConfig config) {}
    virtual void stop() {}

    inline void init(unsigned long baud) {
        init(baud, SerialConfig::Config_8N1);
    }

    inline size_t available() const {
        return _readBuffer.available();
    }

    inline virtual uint8_t read() {
        while (_readBuffer.isEmpty()) ;
        return _readBuffer.read();
    }

    inline void read(uint8_t* buf, size_t len) {
        for (unsigned int i = 0; i < len; ++i) {
            buf[i] = read();
        }
    }

    inline virtual void write(uint8_t x) {
        while (_writeBuffer.isFull()) ;
        _writeBuffer.write(x);
    }

    inline void write(uint8_t* buf, size_t len) {
        for (unsigned int i = 0; i < len; ++i) {
            write(buf[i]);
        }
    }

protected:
    CircularBuffer _readBuffer;
    CircularBuffer _writeBuffer;
};


/** Uses the hardware implementation of UART to provide a user-friendly buffered serial stream. */
class HardwareSerial final : public Serial {
public:
    explicit HardwareSerial(
        volatile uint8_t* ubrrh,
        volatile uint8_t* ubrrl,
        volatile uint8_t* ucsra,
        volatile uint8_t* ucsrb,
        volatile uint8_t* ucsrc,
        volatile uint8_t* udr
    )
    : _ubrrh(ubrrh),
      _ubrrl(ubrrl),
      _ucsra(ucsra),
      _ucsrb(ucsrb),
      _ucsrc(ucsrc),
      _udr(udr)
    {
    }

    using Serial::init;
    void init(unsigned long baud, SerialConfig config) override final;
    void stop() override final;

    using Serial::write;
    inline void write(uint8_t x) override final {
        Serial::write(x);
        *_ucsrb |= (1 << UDRIE0); // Enable the Data Register Empty interrupt
    }

    void __doRxIRQ();
    void __doTxIRQ();

private:
    volatile uint8_t* const _ubrrh;
    volatile uint8_t* const _ubrrl;
    volatile uint8_t* const _ucsra;
    volatile uint8_t* const _ucsrb;
    volatile uint8_t* const _ucsrc;
    volatile uint8_t* const _udr;
};



// Provide an already instantiated HardwareSerial object for all the serials natively available on the mcu.
#ifdef SERIAL_HAVE_SERIAL0
    extern HardwareSerial Serial0;
#endif

} // namespace avr