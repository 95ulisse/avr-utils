#pragma once

#include "avr-utils/private/common.hpp"
#include "avr-utils/utility.hpp"
#include "avr-utils/Timer.hpp"

namespace avr {

namespace detail {

/** Helper to iterate the bits set to 1 of a bitmask. */
template <auto Mask>
struct bitmask_iterator {

    using MaskType = decltype(Mask);
    
    template <typename F>
    static inline void run(F&& f) {
        helper<F, sizeof(MaskType) * 8 - 1>(forward<F>(f));
    }

private:

    template <typename F, MaskType N>
    static inline void helper(F&& f) {
        if constexpr ((Mask & (1 << N)) != 0) {
            f.template operator()<N>();
        }
        if constexpr (N > 0) {
            helper<F, N - 1>(forward<F>(f));
        }
    }

};



/** Common digital I/O on a port. */
template <Port P, uint8_t Mask, PinMode Mode>
class digital_io_port {

    using Traits = port_traits<P>;

    struct initialize_timer_for_pwm {
        template <uint8_t i>
        inline void operator()() {
            // Initialize the corresponding timer to Fast PWM with prescaler 64
            using T = typename timer_for_pin<P, 1 << i>::timer;
            T::template setMode<TimerMode::FastPWM>();
            T::template setPrescaler<TimerPrescaler::By64>();
        }
    };

    struct update_pwm_value {

        update_pwm_value(uint8_t v)
            : _value(v)
        {
        }

        template <uint8_t i>
        inline void operator()() {
            constexpr auto mask = 1 << i;
            using T = typename timer_for_pin<P, mask>::timer;
            constexpr auto channel = timer_for_pin<P, mask>::channel;

            // Handle the spacific 0% and 100% duty cicles as common digital writes
            if (_value == 0 || _value == 255) {
                T::template stopOutput<channel>();
                if (_value == 0) {
                    Traits::outputRegister() &= ~mask;
                } else {
                    Traits::outputRegister() |= mask;
                }
            } else {
                T::template startOutput<channel>();
                T::template setOutputCompareValue<channel>(_value);
            }
        }

    private:
        const uint8_t _value;
    };

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
            bitmask_iterator<Mask>::run(initialize_timer_for_pwm{});
            
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
        bitmask_iterator<Mask>::run(update_pwm_value{ value });
    }

};



/** Groups a list of pins by port and mode, yielding a avr::list of pins with an aggregated mask for each combination. */
template <typename... Pins>
class group_by_port_and_mode {

    template <typename P1, typename P2>
    static constexpr bool is_match = P1::port == P2::port && P1::mode == P2::mode;

    template <typename L, typename Pin, typename = void>
    struct aggregate_mask_or_append;

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<is_same_v<L, list<>>>> {
        using type = typename L::template append<Pin>;
    };

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<is_match<typename L::head, Pin>>> {
        using newpin = digital_io_port<Pin::port, L::head::mask | Pin::mask, Pin::mode>;
        using type = typename L::tail::template prepend<newpin>;
    };

    template <typename L, typename Pin>
    struct aggregate_mask_or_append<L, Pin, enable_if_t<!is_match<typename L::head, Pin>>> {
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
    using groups = typename detail::group_by_port_and_mode<Pins...>::type;

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

    // PWM

    static inline void PWM(uint8_t value) {
        groups::for_each([value](auto x) {
            decltype(x)::PWM(value);
        });
    }

};

} // namespace avr