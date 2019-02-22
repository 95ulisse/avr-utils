#pragma once

#include <stdlib.h>

namespace avr {



template <typename T> struct remove_reference      { using type = T; };
template <typename T> struct remove_reference<T&>  { using type = T; };
template <typename T> struct remove_reference<T&&> { using type = T; };



template <typename T1, typename T2> struct is_same { static inline constexpr bool value = false; };
template <typename T> struct is_same<T, T> { static inline constexpr bool value = true; };
template <typename T1, typename T2> static inline constexpr bool is_same_v = is_same<T1, T2>::value;



template <bool B, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> { using type = T; };
template <bool B, typename T = void> using enable_if_t = typename enable_if<B, T>::type;



template <typename T> struct is_enum { static inline constexpr bool value = __is_enum(T); };
template <typename T> static inline constexpr bool is_enum_v = is_enum<T>::value;



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