/**
 * A very simple metaprogramming-friendly tuple implementation with just direct access to components.
 */

#pragma once

#include <stdlib.h>

namespace avr {

// Tuple definition

template <typename T, typename... Ts>
struct Tuple : public Tuple<Ts...> {
    Tuple() : Tuple<Ts...>() {}
    Tuple(T value, Ts... rest) : Tuple<Ts...>(rest...), value(value) {}

    T value;

    inline constexpr size_t Size() const {
        return 1 + sizeof...(Ts);
    }
};

template <typename T>
struct Tuple<T> {
    Tuple() {}
    Tuple(T value) : value(value) {}

    T value;

    inline constexpr size_t Size() const {
        return 1;
    }
};



// get<N>() function

namespace __details {

template <size_t N, typename TFirst, typename... Ts>
struct GetHelper {
    static auto get(const Tuple<TFirst, Ts...>& t) -> decltype(GetHelper<N - 1, Ts...>::get(t)) {
        return GetHelper<N - 1, Ts...>::get(t);
    }
    static auto get(Tuple<TFirst, Ts...>& t) -> decltype(GetHelper<N - 1, Ts...>::get(t)) {
        return GetHelper<N - 1, Ts...>::get(t);
    }
};

template <typename TFirst, typename... Ts>
struct GetHelper<0, TFirst, Ts...> {
    static const TFirst& get(const Tuple<TFirst, Ts...>& t) { return t.value; }
    static TFirst& get(Tuple<TFirst, Ts...>& t) { return t.value; }
};

} // namespace __details

template <size_t N, typename... Ts>
auto get(const Tuple<Ts...>& t) -> decltype(__details::GetHelper<N, Ts...>::get(t)) {
    return __details::GetHelper<N, Ts...>::get(t);
}

template <size_t N, typename... Ts>
auto get(Tuple<Ts...>& t) -> decltype(__details::GetHelper<N, Ts...>::get(t)) {
    return __details::GetHelper<N, Ts...>::get(t);
}

} // namespace avr