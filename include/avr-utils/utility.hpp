#pragma once

#include <stdlib.h>
#include <inttypes.h>



// Placement new
inline void* operator new (size_t n, void* ptr) { return ptr; }



namespace avr {



template <typename T> struct remove_reference      { using type = T; };
template <typename T> struct remove_reference<T&>  { using type = T; };
template <typename T> struct remove_reference<T&&> { using type = T; };
template <typename T> using remove_reference_t = typename remove_reference<T>::type;



template <typename T> struct remove_const           { using type = T; };
template <typename T> struct remove_const<const T>  { using type = T; };
template <typename T> using remove_const_t = typename remove_const<T>::type;



template <typename T> struct remove_volatile              { using type = T; };
template <typename T> struct remove_volatile<volatile T>  { using type = T; };
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;



template <typename T>
struct remove_cv {
    using type = typename remove_volatile<typename remove_const<T>::type>::type;
};
template <typename T> using remove_cv_t = typename remove_cv<T>::type;



template <bool C, typename T, typename F> struct conditional { using type = F; };
template <typename T, typename F> struct conditional<true, T, F> { using type = T; };
template <bool C, typename T, typename F> using conditional_t = typename conditional<C, T, F>::type;



template <typename T1, typename T2> struct is_same { static constexpr bool value = false; };
template <typename T> struct is_same<T, T> { static constexpr bool value = true; };
template <typename T1, typename T2> static constexpr bool is_same_v = is_same<T1, T2>::value;



template <bool B, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> { using type = T; };
template <bool B, typename T = void> using enable_if_t = typename enable_if<B, T>::type;



template <typename T> struct is_enum { static constexpr bool value = __is_enum(T); };
template <typename T> static constexpr bool is_enum_v = is_enum<T>::value;



template <typename T> struct is_trivially_copyable { static constexpr bool value = __is_trivially_copyable(T); };
template <typename T> static constexpr bool is_trivially_copyable_v = is_trivially_copyable<T>::value;



template <typename T> struct underlying_type { using type = __underlying_type(T); };
template <typename T> using underlying_type_t = typename underlying_type<T>::type;



template <typename T>
constexpr T&& forward(typename remove_reference<T>::type& x) noexcept {
    return static_cast<T&&>(x);
}

template <typename T>
constexpr T&& forward(typename remove_reference<T>::type&& x) noexcept {
    return static_cast<T&&>(x);
}



template <typename T>
constexpr typename remove_reference<T>::type&& move(T&& x) noexcept {
    return static_cast<typename remove_reference<T>::type&&>(x);
}



template <typename...>
struct list;

template <typename Head, typename... Rest>
struct list<Head, Rest...> {
    using head = Head;
    using tail = list<Rest...>;

    template <typename T> using append = list<Head, Rest..., T>;
    template <typename T> using prepend = list<T, Head, Rest...>;

    template <typename F>
    static inline void for_each(F&& f) {
        f(Head());
        tail::for_each(forward<F>(f));
    }
};

template <> struct list<> {
    template <typename T> using append = list<T>;
    template <typename T> using prepend = list<T>;

    template <typename F>
    static inline void for_each(F&&) {}
};



template <typename T, T... Ints> struct integer_sequence {
    using value_type = T;
    static constexpr size_t size() noexcept { return sizeof...(Ints); }
};

namespace detail {

template <typename T> struct identity { using type = T; };

template <typename T, T N, T... Pack>
struct make_integer_sequence_helper {
    using type = typename conditional_t<
        N == T(0),
        identity<integer_sequence<T, Pack...>>,
        make_integer_sequence_helper<T, N - 1, N - 1, Pack...>
    >::type;
};

} // namespace detail

template <typename T, T N> using make_integer_sequence = typename detail::make_integer_sequence_helper<T, N>::type;

template <size_t... Ints> using index_sequence = integer_sequence<size_t, Ints...>;
template <size_t N> using make_index_sequence = make_integer_sequence<size_t, N>;



template <size_t X, size_t... Rest>
struct static_max {
    static constexpr size_t value = X > static_max<Rest...>::value ? X : static_max<Rest...>::value;
};

template <size_t X>
struct static_max<X> {
    static constexpr size_t value = X;
};



template <size_t N, size_t Align>
struct aligned_storage {
    struct type {
        alignas(Align) unsigned char data[N];
    };
};



// Min/Max functions

template <typename T>
static inline const T& min(const T& a, const T& b) { return (a < b) ? a : b; }

template <typename T>
static inline const T& max(const T& a, const T& b) { return (a < b) ? b : a; }



// Endianess conversion functions

static inline constexpr uint16_t htons(uint16_t x) {
    return (x << 8 & 0xFF00) |
           (x >> 8 & 0x00FF);
}

static inline constexpr uint16_t ntohs(uint16_t x) {
    return htons(x);
}

static inline constexpr uint32_t htonl(uint32_t x) {
    return (x << 24 & 0xFF000000UL) |
           (x << 8  & 0x00FF0000UL) |
           (x >> 8  & 0x0000FF00UL) |
           (x >> 24 & 0x000000FFUL);
}

static inline constexpr uint32_t ntohl(uint32_t x) {
    return htonl(x);
}

} // namespace avr
