#pragma once

#include "avr-utils/private/common.hpp"
#include "avr-utils/Utility.hpp"
#include "avr-utils/Timer.hpp"

namespace avr {

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
        } else if constexpr (Mode == PinMode::PWM) {
            
            // Set the direction to output
            Traits::dataDirectionRegister() |= Mask;

            // Initialize the corresponding timer to Fast PWM with prescaler 64.
            // This fails if this pin has no attached timer.
            using T = typename timer_for_pin<P, Mask>::timer;
            T::template setMode<TimerMode::FastPWM>();
            T::template setPrescaler<TimerPrescaler::By64>();
            
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

    // PWM

    static inline void PWM(uint8_t value) {
        static_assert(Mode == PinMode::PWM);

        // Handle the spacific 0% and 100% duty cicles as common digital writes
        using T = typename timer_for_pin<P, Mask>::timer;
        constexpr auto channel = timer_for_pin<P, Mask>::channel;
        if (value == 0 || value == 255) {
            T::template stopOutput<channel>();
            if (value == 0) {
                unset();
            } else {
                set();
            }
        } else {
            T::template startOutput<channel>();
            T::setOutputCompareValue(value);
        }
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