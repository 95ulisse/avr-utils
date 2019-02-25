#pragma once

#include <inttypes.h>

#include "avr-utils/utility.hpp"

namespace avr {

template <auto T>
struct Field;



namespace __detail {

template <typename T, typename Enable = void>
struct ValueSerializer;

template <>
struct ValueSerializer<uint8_t> {
    
    template <typename TIter>
    inline static bool deserialize(uint8_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        *value = *it++;
        return true;
    }

    template <typename TIter>
    inline static bool serialize(const uint8_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        *it++ = *value;
        return true;
    }

};

template <>
struct ValueSerializer<uint16_t> {

    // We only support Big Endian integers

    template <typename TIter>
    inline static bool deserialize(uint16_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        *value = ((uint16_t) (*it++ & 0xFF) << 8);

        if (it == end) {
            return false;
        }
        *value |= (*it++ & 0xFF);

        return true;
    }

    template <typename TIter>
    inline static bool serialize(const uint16_t* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        *it++ = (uint8_t) ((*value >> 8) & 0xFF);
        
        if (it == end) {
            return false;
        }
        *it++ = (uint8_t) ((*value) & 0xFF);

        return true;
    }

};

template <>
struct ValueSerializer<uint32_t> {

    // We only support Big Endian integers

    template <typename TIter>
    inline static bool deserialize(uint32_t* value, TIter&& it, TIter&& end) {
        *value = 0;
        for (int i = 3; i >= 0; --i) {
            if (it == end) {
                return false;
            }
            *value |= ((uint32_t) (*it++ & 0xFF) << (i * 8));
        }
        return true;
    }

    template <typename TIter>
    inline static bool serialize(const uint32_t* value, TIter&& it, TIter&& end) {
        for (int i = 3; i >= 0; --i) {
            if (it == end) {
                return false;
            }
            *it++ = (uint8_t) ((*value >> (i * 8)) & 0xFF);
        }
        return true;
    }

};

template <>
struct ValueSerializer<bool> {

    // Yep, each bool takes a whole byte

    template <typename TIter>
    inline static bool deserialize(bool* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        uint8_t x = *it++;
        if (x != 0 && x != 1) {
            return false;
        }
        *value = x == 1;
        return true;
    }

    template <typename TIter>
    inline static bool serialize(const bool* value, TIter&& it, TIter&& end) {
        if (it == end) {
            return false;
        }
        *it++ = (uint8_t) (*value ? 1 : 0);
        return true;
    }

};

template <typename T, size_t N>
struct ValueSerializer<T[N]> {

    // Serializer forarrays with compile-time known length delegate to the underlying type

    template <typename TIter>
    inline static bool deserialize(T (*value)[N], TIter&& it, TIter&& end) {
        for (unsigned int i = 0; i < N; ++i) {
            if (!ValueSerializer<T>::deserialize(&((*value)[i]), forward<TIter>(it), forward<TIter>(end))) {
                return false;
            }
        }
        return true;
    }

    template <typename TIter>
    inline static bool serialize(const T (*value)[N], TIter&& it, TIter&& end) {
        for (unsigned int i = 0; i < N; ++i) {
            if (!ValueSerializer<T>::serialize(&((*value)[i]), forward<TIter>(it), forward<TIter>(end))) {
                return false;
            }
        }
        return true;
    }

};

template <typename T>
struct ValueSerializer<T, enable_if_t<is_enum_v<T>>> {
    
    // Delegate enums to their underlying type
    using U = underlying_type_t<T>;

    template <typename TIter>
    inline static bool deserialize(T* value, TIter&& it, TIter&& end) {
        return ValueSerializer<U>::deserialize(reinterpret_cast<U*>(value), forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static bool serialize(const T* value, TIter&& it, TIter&& end) {
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
    inline static bool deserialize(T* value, TIter&& it, TIter&& end) {
        return S::deserialize(*value, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static bool serialize(const T* value, TIter&& it, TIter&& end) {
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
    inline static bool deserialize(typename F::ContainerType& obj, TIter&& it, TIter&& end) {
        if (F::deserialize(obj, forward<TIter>(it), forward<TIter>(end))) {
            return combine_fields<Rest...>::deserialize(obj, forward<TIter>(it), forward<TIter>(end));
        }
        return false;
    }

    template <typename TIter>
    inline static bool serialize(const typename F::ContainerType& obj, TIter&& it, TIter&& end) {
        if (F::serialize(obj, forward<TIter>(it), forward<TIter>(end))) {
            return combine_fields<Rest...>::serialize(obj, forward<TIter>(it), forward<TIter>(end));
        }
        return false;
    }

};

template <typename F>
struct combine_fields<F> {
    
    using ContainerType = typename F::ContainerType;

    template <typename TIter>
    inline static bool deserialize(ContainerType& obj, TIter&& it, TIter&& end) {
        return F::deserialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static bool serialize(const ContainerType& obj, TIter&& it, TIter&& end) {
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
    inline static bool deserialize(TContainer& obj, TIter&& it, TIter&& end) {
        return __detail::ValueSerializer<TValue>::deserialize(&(obj.*member), forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static bool serialize(const TContainer& obj, TIter&& it, TIter&& end) {
        return __detail::ValueSerializer<TValue>::serialize(&(obj.*member), forward<TIter>(it), forward<TIter>(end));
    }
};



/** Combined serializer for a list of fields. */
template <typename... Fields>
class Serializer {

    using CombinedFields = __detail::combine_fields<Fields...>;

public:

    template <typename TIter>
    inline static bool deserialize(typename CombinedFields::ContainerType& obj, TIter&& it, TIter&& end) {
        return CombinedFields::deserialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

    template <typename TIter>
    inline static bool serialize(const typename CombinedFields::ContainerType& obj, TIter&& it, TIter&& end) {
        return CombinedFields::serialize(obj, forward<TIter>(it), forward<TIter>(end));
    }

};

// Specialization to handle empty structures
template <>
class Serializer<> {
public:

    template <typename TContainer, typename TIter>
    inline static bool deserialize(TContainer& obj, TIter&& it, TIter&& end) {
        return true;
    }

    template <typename TContainer, typename TIter>
    inline static bool serialize(const TContainer& obj, TIter&& it, TIter&& end) {
        return true;
    }

};

} // namespace avr