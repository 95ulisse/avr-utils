#pragma once

#include "Utility.hpp"

namespace avr {

enum class Port {
    A, B, C, D, E, F
};

enum class PinMode {
    Input,
    InputPullup,
    Output
};



template <Port P>
struct port_traits;

// Automatically implement all the specializations of port_traits
// based on the macros defined by the compiler

#define AVR_UTILS_SPECIALIZE_PORT_TRAITS(p)                                      \
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


namespace detail {

/** Common digital I/O on a port. */
template <Port P, uint8_t Mask, PinMode Mode>
class digital_io_port {

    using Traits = port_traits<P>;

public:

    static constexpr auto port = P;
    static constexpr auto mask = Mask;
    static constexpr auto mode = Mode;

    // Pin initialization

    __attribute__((always_inline))
    static inline void init() {
        if constexpr (Mode == PinMode::Output) {
            Traits::dataDirectionRegister() |= Mask;
        } else if constexpr (Mode == PinMode::Input) {
            Traits::dataDirectionRegister() &= ~Mask;
        } else if constexpr (Mode == PinMode::InputPullup) {
            Traits::dataDirectionRegister() &= ~Mask;
            Traits::outputRegister() |= Mask; // Enable the pullup by default
        }
    }

    // Digital read

    __attribute__((always_inline))
    static inline bool read() {
        static_assert(Mode == PinMode::Input || Mode == PinMode::InputPullup);
        return (Traits::inputRegister() & Mask) != 0;
    }

    // Digital write

    __attribute__((always_inline))
    static inline void set() {
        static_assert(Mode == PinMode::Output);
        Traits::outputRegister() |= Mask;
    }

    __attribute__((always_inline))
    static inline void unset() {
        static_assert(Mode == PinMode::Output);
        Traits::outputRegister() &= ~Mask;
    }

    __attribute__((always_inline))
    static inline void toggle() {
        static_assert(Mode == PinMode::Output);
        Traits::outputRegister() ^= Mask;
    }

};



/** Groups a list of pins by port, yielding a avr::list of pins with an aggregated mask for each port. */
template <typename... Pins>
class group_by_port {

    template <typename L, typename Pin, typename = void>
    struct aggregate_mask_or_append;

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<is_same_v<L, list<>>>> {
        using type = typename L::template append<Pin>;
    };

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<L::head::port == Pin::port>> {
        using newpin = digital_io_port<Pin::port, L::head::mask | Pin::mask, Pin::mode>;
        using type = typename L::tail::template prepend<newpin>;
    };

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<L::head::port != Pin::port>> {
        using type =
            typename aggregate_mask_or_append<
                typename L::tail,
                Pin
            >::type
            ::template prepend<typename L::head>;
    };



    template <typename L, typename P, typename... Rest>
    struct helper {
        using tmp = typename aggregate_mask_or_append<L, P>::type;
        using type = typename helper<tmp, Rest...>::type;
    };

    template <typename L, typename P>
    struct helper<L, P> {
        using type = typename aggregate_mask_or_append<L, P>::type;
    };

public:

    using type = typename helper<list<>, Pins...>::type;

};

}



/**
 * Very simple abstraction over a single IO pin.
 * It basically just wraps the operations with the registers in a more friendly interface.
 */
template <Port Port, uint8_t N, PinMode Mode>
using Pin = detail::digital_io_port<Port, 1 << N, Mode>;



/**
 * Group of pins to optimize collective operations.
 * Can also hold pins of different ports and modes, but only writes can be performed on them as a group.
 */
template <typename... Pins>
class PinGroup {

    // Group the pins by port
    using groups = typename detail::group_by_port<Pins...>::type;

public:

    // Initialization

    __attribute__((always_inline))
    static inline void init() {
        groups::for_each([](auto x) {
            decltype(x)::init();
        });
    }

    // Digital write

    __attribute__((always_inline))
    static inline void set() {
        groups::for_each([](auto x) {
            decltype(x)::set();
        });
    }

    __attribute__((always_inline))
    static inline void unset() {
        groups::for_each([](auto x) {
            decltype(x)::unset();
        });
    }

    __attribute__((always_inline))
    static inline void toggle() {
        groups::for_each([](auto x) {
            decltype(x)::toggle();
        });
    }

};

} // namespace avr