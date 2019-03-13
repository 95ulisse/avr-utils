#pragma once

#include <stdlib.h>
#include <inttypes.h>

#include "avr-utils/utility.hpp"
#include "avr-utils/Serializable.hpp"

namespace avr {



// Forward declaration
template <typename... Ts> class Variant;



namespace __detail {

template <typename... Ts>
struct tag_list {

    template <typename T>
    static constexpr size_t get() {
        return getHelper<T, 1, Ts...>::value;
    }

    static constexpr size_t invalid = 0;

private:

    template <typename Target, size_t N, typename... Rest>
    struct getHelper;

    template <typename Target, size_t N, typename... Rest>
    struct getHelper<Target, N, Target, Rest...> {
        static constexpr size_t value = N;
    };

    template <typename Target, size_t N, typename Other, typename... Rest>
    struct getHelper<Target, N, Other, Rest...> {
        static constexpr size_t value = getHelper<Target, N + 1, Rest...>::value;
    };

};



template <typename TVariant, typename... Rest>
struct variant_helper;

template <typename TVariant, typename T, typename... Rest>
struct variant_helper<TVariant, T, Rest...> {

    inline static void destroy(uint8_t tag, void* storage) {
        if (tag == TVariant::Tags::template get<T>()) {
            reinterpret_cast<T*>(storage)->~T();
        } else {
            variant_helper<TVariant, Rest...>::destroy(tag, storage);
        }
    }

    inline static void copy(uint8_t otherTag, void* otherStorage, void* storage) {
        if (otherTag == TVariant::Tags::template get<T>()) {
            new (storage) T(*reinterpret_cast<T*>(otherStorage));
        } else {
            variant_helper<TVariant, Rest...>::copy(otherTag, otherStorage, storage);
        }
    }

    template <typename TVisitor>
    inline static void visit(uint8_t tag, void* storage, TVisitor&& visitor) {
        if (tag == TVariant::Tags::template get<T>()) {
            visitor(*reinterpret_cast<T*>(storage));
        } else {
            variant_helper<TVariant, Rest...>::visit(tag, storage, forward<TVisitor>(visitor));
        }
    }

    template <typename TVisitor>
    inline static void visit(uint8_t tag, const void* storage, TVisitor&& visitor) {
        if (tag == TVariant::Tags::template get<T>()) {
            visitor(*reinterpret_cast<const T*>(storage));
        } else {
            variant_helper<TVariant, Rest...>::visit(tag, storage, forward<TVisitor>(visitor));
        }
    }

    template <typename TIter>
    inline static Optional<size_t> deserialize(uint8_t tag, TVariant& v, TIter&& it, TIter&& end) {
        if (tag == TVariant::Tags::template get<T>()) {
            v.template emplace<T>();
            auto& x = v.template get<T>();
            return ValueSerializer<T>::deserialize(&x, forward<TIter>(it), forward<TIter>(end));
        } else {
            return variant_helper<TVariant, Rest...>::deserialize(tag, v, forward<TIter>(it), forward<TIter>(end));
        }
    }
    
    template <typename TIter>
    inline static Optional<size_t> serialize(const TVariant& v, TIter&& it, TIter&& end) {
        if (v.template is<T>()) {
            auto& x = v.template get<T>();
            return ValueSerializer<T>::serialize(&x, forward<TIter>(it), forward<TIter>(end));
        } else {
            return variant_helper<TVariant, Rest...>::serialize(v, forward<TIter>(it), forward<TIter>(end));
        }
    }

};

template <typename TVariant>
struct variant_helper<TVariant> {

    inline static void destroy(uint8_t tag, void* storage) {}

    inline static void copy(uint8_t otherTag, void* otherStorage, void* storage) {}

    template <typename TVisitor>
    inline static void visit(uint8_t tag, void* storage, TVisitor&& visitor) {}

    template <typename TVisitor>
    inline static void visit(uint8_t tag, const void* storage, TVisitor&& visitor) {}

    template <typename TIter>
    inline static Optional<size_t> deserialize(uint8_t, TVariant&, TIter&&, TIter&&) {
        return nullopt;
    }
    
    template <typename TIter>
    inline static Optional<size_t> serialize(const TVariant&, TIter&&, TIter&&) {
        return nullopt;
    }

};



template <typename... Ts>
struct visitor : public Ts... {
    visitor(const Ts&... args) : Ts(args)... {}

    using Ts::operator()...;
};



template <typename TVariant, typename Helper>
struct variant_serializer {
    
    template <typename TIter>
    inline static Optional<size_t> deserialize(TVariant& value, TIter&& it, TIter&& end) {
        
        // First the tag
        if (it == end) {
            return nullopt;
        }
        uint8_t tag = *it++;

        // Then the value
        if (auto res = Helper::deserialize(tag, value, forward<TIter>(it), forward<TIter>(end))) {
            return *res + 1;
        } else {
            return nullopt;
        }
        
    }

    template <typename TIter>
    inline static Optional<size_t> serialize(const TVariant& value, TIter&& it, TIter&& end) {
        if (value.isInvalid() || it == end) {
            return nullopt;
        }

        // First the tag
        *it++ = value.tag();

        // Then the actual value
        if (auto res = Helper::serialize(value, forward<TIter>(it), forward<TIter>(end))) {
            return *res + 1;
        } else {
            return nullopt;
        }
        
    }

};

} // namespace __detail



/**
 * A barely usable type-safe tagged union.
 * This is a very simple variant. No exception safety, no repeated types, no move semantics.
 */
template <typename... Ts>
class Variant {

    static_assert(sizeof...(Ts) >= 1, "At least one variant must be present.");
    static_assert(sizeof...(Ts) <= 255, "Cannot have more than 255 possible variants.");

    // The storage for the members must be able to contain the biggest member
    using TStorage = typename aligned_storage<
        static_max<sizeof(Ts)...>::value,
        static_max<alignof(Ts)...>::value
    >::type;

    using Helper = __detail::variant_helper<Variant<Ts...>, Ts...>;

    uint8_t _tag; // All tags start from 1
    TStorage _storage;

public:

    // As tags we use the index inside the template pack Ts.
    using Tags = __detail::tag_list<Ts...>;

    // Serializer type for automatic serialization of variants
    using Serializer = __detail::variant_serializer<Variant<Ts...>, Helper>;

    Variant()
        : _tag(Tags::invalid)
    {
    }

    template <typename T>
    Variant(T&& val)
        : _tag(Tags::invalid)
    {
        set(forward<T>(val));
    }

    Variant(Variant<Ts...>&& other) = delete; // Non-movable

    Variant(const Variant<Ts...>& other)
        : _tag(other.tag)
    {
        Helper::copy(other._tag, &other._storage, &_storage);
    }

    ~Variant() {
        Helper::destroy(_tag, &_storage);
    }

    // Only copy assigment
    Variant<Ts...>& operator=(const Variant<Ts...>& other) {
        if (this != other) {

            // Destroy our value
            Helper::destroy(_tag, &_storage);

            // Copy the value of the other variant into our storage
            Helper::copy(other._tag, &other._storage, &_storage);
            _tag = other._tag;

        }
        return *this;
    }

    template <typename T>
    inline T& get() {
        // Make sure that we are accessing the right value
        if (isInvalid() || !is<T>()) {
            abort();
        } else {
            return *reinterpret_cast<T*>(&_storage);
        }
    }

    template <typename T>
    inline const T& get() const {
        // Make sure that we are accessing the right value
        if (isInvalid() || !is<T>()) {
            abort();
        } else {
            return *reinterpret_cast<const T*>(&_storage);
        }
    }

    template <typename T>
    inline bool is() const {
        return _tag == Tags::template get<T>();
    }

    inline bool isInvalid() const {
        return _tag == Tags::invalid;
    }

    inline uint8_t tag() const {
        return _tag;
    }

    template <typename T, typename... Args>
    void emplace(Args&&... args) {

        // First, destroy the current value
        Helper::destroy(_tag, &_storage);

        // Construct the new value in the preallocated storage
        new (&_storage) T(forward<Args>(args)...);
        _tag = Tags::template get<T>();

    }

    template <typename T>
    void set(T&& arg) {
        
        // First, destroy the current value
        Helper::destroy(_tag, &_storage);

        // Copy or move the value in the storage
        using T_noreference = typename remove_reference<T>::type;
        new (&_storage) T_noreference(forward<T>(arg));
        _tag = Tags::template get<T_noreference>();
        
    }

    template <typename... Funcs>
    void visit(const Funcs&... funcs) {

        // Cannot visit an invalid variant
        if (isInvalid()) {
            abort();
        }

        // Build a visitor
        __detail::visitor<Funcs...> visitor(funcs...);

        // Invoke the correct overload of operator() based on the current tag
        Helper::visit(_tag, &_storage, visitor);

    }

    template <typename... Funcs>
    void visit(const Funcs&... funcs) const {

        // Cannot visit an invalid variant
        if (isInvalid()) {
            abort();
        }

        // Build a visitor
        __detail::visitor<Funcs...> visitor(funcs...);

        // Invoke the correct overload of operator() based on the current tag
        Helper::visit(_tag, &_storage, visitor);

    }

};

} // namespace avr
