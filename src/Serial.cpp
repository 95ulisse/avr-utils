#include "Serial.hpp"

namespace avr {

void HardwareSerial::init(unsigned long baud, SerialConfig config) {

    // First try to use U2X
    uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
    *_ucsra = 1 << U2X0;

    // But fall back to non-U2X if baud rate is too low
    if (baud_setting > 4095) {
        *_ucsra = 0;
        baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }

    *_ubrrh = baud_setting >> 8;
    *_ubrrl = baud_setting;
    *_ucsrc = static_cast<uint8_t>(config);

    // Enable TX and RX and interrupts
    *_ucsrb = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    sei();

}

void HardwareSerial::stop() {
    // Stop both RX and TX and interrupts
    *_ucsrb = 0;
}

void HardwareSerial::__doRxIRQ() {
    // Store the received byte in the buffer if no error has happened
    if ((*_ucsra & (1 << UPE0)) == 0) {
        if (!_readBuffer.isFull()) {
            _readBuffer.write(*_udr);
        }
    } else {
        *_udr; // Read and discard
    }
}

void HardwareSerial::__doTxIRQ() {
    // Since the interrupt is enabled, there must be some data in the tx buffer
    uint8_t c = _writeBuffer.read();
    *_udr = c;

    // If we emptied the buffer, disable the interrupt
    if (_writeBuffer.isEmpty()) {
        *_ucsrb &= ~(1 << UDRIE0);
    }
}



#if defined(UBRR0H)
HardwareSerial Serial0(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);

ISR(USART_RX_vect) {
    Serial0.__doRxIRQ();
}

ISR(USART_UDRE_vect) {
    Serial0.__doTxIRQ();
}
#endif

} // namespace avr