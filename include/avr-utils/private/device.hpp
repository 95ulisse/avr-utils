#pragma once

#include "avr-utils/private/common.hpp"

namespace avr {

// Automatically implement all the specializations of port_traits
// based on the macros defined by the compiler

#define AVR_UTILS_SPECIALIZE_PORT_TRAITS(p)                                   \
    template<> struct port_traits<Port::p> {                                  \
        static volatile uint8_t& dataDirectionRegister() { return DDR ##p; }  \
        static volatile uint8_t& outputRegister() { return PORT ##p; }        \
        static volatile uint8_t& inputRegister() { return PIN ##p; }          \
    };

#ifdef PORTA
AVR_UTILS_SPECIALIZE_PORT_TRAITS(A)
#endif
#ifdef PORTB
AVR_UTILS_SPECIALIZE_PORT_TRAITS(B)
#endif
#ifdef PORTC
AVR_UTILS_SPECIALIZE_PORT_TRAITS(C)
#endif
#ifdef PORTD
AVR_UTILS_SPECIALIZE_PORT_TRAITS(D)
#endif
#ifdef PORTE
AVR_UTILS_SPECIALIZE_PORT_TRAITS(E)
#endif
#ifdef PORTF
AVR_UTILS_SPECIALIZE_PORT_TRAITS(F)
#endif

#undef AVR_UTILS_SPECIALIZE_PORT_TRAITS

} // namespace avr



// Timer mappings are dependent of the current microcontroller,
// so include the file with the appropriate definitions.

#if defined(__AVR_ATmega48A__) || defined(__AVR_ATmega48PA__) || \
    defined(__AVR_ATmega88A__) || defined(__AVR_ATmega88PA__) || \
    defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__) || \
    defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#   include "avr-utils/private/device/atmegaxx8.hpp"
#else
#   error "Unsupported microcontroller."
#endif