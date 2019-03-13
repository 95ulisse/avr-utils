#pragma once

#include <inttypes.h>

#include "avr-utils/utility.hpp"
#include "avr-utils/Optional.hpp"

namespace avr {

template <auto T>
struct Field;



namespace __detail {

template <typename T, typename Enable = void>
struct ValueSerializer;

template <>
struct ValueSerializer<uint8_t> {
    
    template <typename TIter>
    inline static Optional<size_t> deserialize(uint8_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        *value = *it++;
        return 1;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const uint8_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        *it++ = *value;
        return 1;
    }

};

template <>
struct ValueSerializer<uint16_t> {

    // We only support Big Endian integers

    template <typename TIter>
    inline static Optional<size_t> deserialize(uint16_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        *value = ((uint16_t) (*it++ & 0xFF) << 8);

        if (it == end) {
            return nullopt;
        }
        *value |= (*it++ & 0xFF);

        return 2;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const uint16_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        *it++ = (uint8_t) ((*value >> 8) & 0xFF);
        
        if (it == end) {
            return nullopt;
        }
        *it++ = (uint8_t) ((*value) & 0xFF);

        return 2;
    }

};

template <>
struct ValueSerializer<uint32_t> {

    // We only support Big Endian integers

    template <typename TIter>
    inline static Optional<size_t> deserialize(uint32_t* value, TIter&& it, TIter&& end) {
        *value = 0;
        for (int i = 3; i >= 0; --i) {
            if (it == end) {
                return nullopt;
            }
            *value |= ((uint32_t) (*it++ & 0xFF) << (i * 8));
        }
        return 4;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const uint32_t* value, TIter&& it, TIter&& end) {
        for (int i = 3; i >= 0; --i) {
            if (it == end) {
                return nullopt;
            }
            *it++ = (uint8_t) ((*value >> (i * 8)) & 0xFF);
        }
        return 4;
    }

};

template <>
struct ValueSerializer<bool> {

    // Yep, each bool takes a whole byte

    template <typename TIter>
    inline static Optional<size_t> deserialize(bool* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        uint8_t x = *it++;
        if (x != 0 && x != 1) {
            return nullopt;
        }
        *value = x == 1;
        return 1;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const bool* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return nullopt;
        }
        *it++ = (uint8_t) (*value ? 1 : 0);
        return 1;
    }

};

template <typename T, size_t N>
struct ValueSerializer<T[N]> {

    // Serializer for arrays with compile-time known length delegate to the underlying type

    template <typename TIter>
    inline static Optional<size_t> deserialize(T (*value)[N], TIter&& it, TIter&& end) {
        size_t total = 0;
        for (unsigned int i = 0; i < N; ++i) {
            if (auto res = ValueSerializer<T>::deserialize(&((*value)[i]), forward<TIter>(it), forward<TIter>(end))) {
                total += *res;
            } else {
                return nullopt;
            }
        }
        return total;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const T (*value)[N], TIter&& it, TIter&& end) {
        size_t total = 0;
        for (unsigned int i = 0; i < N; ++i) {
            if (auto res = ValueSerializer<T>::serialize(&((*value)[i]), forward<TIter>(it), forward<TIter>(end))) {
                total += *res;
            } else {
                return nullopt;
            }
        }
        return total;
    }

};

template <typename T>
struct ValueSerializer<T, enable_if_t<is_enum_v<T>>> {
    
    // Delegate enums to their underlying type
    using U = underlying_type_t<T>;

    template <typename TIter>
    inline static Optional<size_t> deserialize(T* value, TIter&& it, TIter&& end) {
        return ValueSerializer<U>::deserialize(reinterpret_cast<U*>(value), forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const T* value, TIter&& it, TIter&& end) {
        return ValueSerializer<U>::serialize(reinterpret_cast<const U*>(value), forward<TIter>(it), forward<TIter>(end));
    }

};

template <typename T>
class has_custom_serializer {

    struct Yes {};
    struct No {};

    template <typename U, typename = typename U::Serializer>
    static Yes check(int);

    template <typename>
    static No check(...);

public:
    static inline const bool value = is_same_v<Yes, decltype(check<T>(0))>;
};

template <typename T>
struct ValueSerializer<T, enable_if_t<has_custom_serializer<T>::value>> {
    
    // Complex types can have serializers for themselves, use them
    using S = typename T::Serializer;

    template <typename TIter>
    inline static Optional<size_t> deserialize(T* value, TIter&& it, TIter&& end) {
        return S::deserialize(*value, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const T* value, TIter&& it, TIter&& end) {
        return S::serialize(*value, forward<TIter>(it), forward<TIter>(end));
    }

};



/** Helper type to sequentially combine fields. */
template <typename... Fields>
struct combine_fields;

template <typename F, typename... Rest>
struct combine_fields<F, Rest...> {

    using ContainerType = typename F::ContainerType;
    static_assert(is_same_v<ContainerType, typename combine_fields<Rest...>::ContainerType>, "Can only combine fields of the same type");

    template <typename TIter>
    inline static Optional<size_t> deserialize(typename F::ContainerType& obj, TIter&& it, TIter&& end) {
        if (auto res1 = F::deserialize(obj, forward<TIter>(it), forward<TIter>(end))) {
            if (auto res2 = combine_fields<Rest...>::deserialize(obj, forward<TIter>(it), forward<TIter>(end))) {
                return (*res1) + (*res2);
            }
        }
        return nullopt;
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const typename F::ContainerType& obj, TIter&& it, TIter&& end) {
        if (auto res1 = F::serialize(obj, forward<TIter>(it), forward<TIter>(end))) {
            if (auto res2 = combine_fields<Rest...>::serialize(obj, forward<TIter>(it), forward<TIter>(end))) {
                return (*res1) + (*res2);
            }
        }
        return nullopt;
    }

};

template <typename F>
struct combine_fields<F> {
    
    using ContainerType = typename F::ContainerType;

    template <typename TIter>
    inline static Optional<size_t> deserialize(ContainerType& obj, TIter&& it, TIter&& end) {
        return F::deserialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const ContainerType& obj, TIter&& it, TIter&& end) {
        return F::serialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

};

} // namespace __detail



/** Descriptor for a single field of a serializable type. */
template <typename TContainer, typename TValue, TValue TContainer::*member>
struct Field<member> {
    using ContainerType = TContainer;
    using ValueType = TValue;

    template <typename TIter>
    inline static Optional<size_t> deserialize(TContainer& obj, TIter&& it, TIter&& end) {
        return __detail::ValueSerializer<TValue>::deserialize(&(obj.*member), forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const TContainer& obj, TIter&& it, TIter&& end) {
        return __detail::ValueSerializer<TValue>::serialize(&(obj.*member), forward<TIter>(it), forward<TIter>(end));
    }
};



/** Combined serializer for a list of fields. */
template <typename... Fields>
class Serializer {

    using CombinedFields = __detail::combine_fields<Fields...>;

public:

    template <typename TIter>
    inline static Optional<size_t> deserialize(typename CombinedFields::ContainerType& obj, TIter&& it, TIter&& end) {
        return CombinedFields::deserialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const typename CombinedFields::ContainerType& obj, TIter&& it, TIter&& end) {
        return CombinedFields::serialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

};

// Specialization to handle empty structures
template <>
class Serializer<> {
public:

    template <typename TContainer, typename TIter>
    inline static Optional<size_t> deserialize(TContainer& obj, TIter&& it, TIter&& end) {
        return 0;
    }

    template <typename TContainer, typename TIter>
    inline static Optional<size_t> serialize(const TContainer& obj, TIter&& it, TIter&& end) {
        return 0;
    }

};

} // namespace avr
