#pragma once

#include "avr-utils/utility.hpp"

namespace avr {

/** Inactive state of an optional. */
struct nullopt_t {
    struct constructor_tag {};
    constexpr explicit nullopt_t(constructor_tag) {}
};
static constexpr nullopt_t nullopt = nullopt_t(nullopt_t::constructor_tag());



/** An optional represents a value that might be present or not. */
template <typename T>
class Optional {
public:

    // Initialization constructors

    constexpr Optional() : Optional(nullopt) {}

    constexpr Optional(nullopt_t) {}

    constexpr Optional(const T& x) { initialize(x); }

    constexpr Optional(T&& x) { initialize(move(x)); }

    // Copy and move constructors

    Optional(const Optional<T>& rhs) {
        if (rhs.has_value()) {
            initialize(*rhs);
        }
    }

    Optional(Optional<T>&& rhs) {
        if (rhs.has_value()) {
            initialize(move(*rhs));
        }
    }

    // Destructor

    ~Optional() { reset(); }

    // Assignment operator
    
    inline Optional& operator=(nullopt_t) {
        reset();
        return *this;
    }

    inline Optional& operator=(const Optional& rhs) {
        if      ( _active && !rhs._active) reset();
        else if ( _active &&  rhs._active) contained_obj() = *rhs;
        else if (!_active &&  rhs._active) initialize(*rhs);

        return *this;
    }

    inline Optional& operator=(Optional&& rhs) {
        if      ( _active && !rhs._active) reset();
        else if ( _active &&  rhs._active) contained_obj() = move(*rhs);
        else if (!_active &&  rhs._active) initialize(move(*rhs));

        return *this;
    }

    inline const Optional& operator=(const Optional&& rhs) {
        if      ( _active && !rhs._active) reset();
        else if ( _active &&  rhs._active) contained_obj() = move(*rhs);
        else if (!_active &&  rhs._active) initialize(move(*rhs));

        return *this;
    }

    // Observers

    inline constexpr operator bool()  const { return _active; }
    inline constexpr bool has_value() const { return _active; }

    inline constexpr const T&  value() const&  { if (!_active) abort(); else return contained_obj(); }
    inline constexpr T&        value() &       { if (!_active) abort(); else return contained_obj(); }
    inline constexpr T&&       value() &&      { if (!_active) abort(); else return move(contained_obj()); };
    inline constexpr const T&& value() const&& { if (!_active) abort(); else return move(contained_obj()); };

    inline constexpr T* operator->() { return ptr(); }

    inline constexpr const T&  operator*() const&  { return contained_obj(); }
    inline constexpr T&        operator*() &       { return contained_obj(); }
    inline constexpr T&&       operator*() &&      { return move(contained_obj()); }
    inline constexpr const T&& operator*() const&& { return move(contained_obj()); }
    
    template <typename V>
    inline constexpr T value_or(V&& v) const& {
        return _active ? contained_obj() : static_cast<T>(v);
    }

    template <typename V>
    inline constexpr T value_or(V&& v) && {
        return _active ? move(contained_obj()) : static_cast<T>(v);
    }

    // Mutators

    template <typename... Args>
    void emplace(Args&&... args) {
        reset();
        initialize(forward<Args>(args)...);
    }

    inline void reset() {
        if (_active) {
            ptr()->~T();
        }
        _active = false;
    }

private:
    bool _active;
    typename aligned_storage<sizeof(T), alignof(T)>::type _storage;

    inline constexpr T* ptr() {
        return reinterpret_cast<T*>(&_storage.data);
    }

    inline constexpr T& contained_obj() &   { return *ptr(); }
    inline constexpr T&& contained_obj() && { return move(*ptr()); }

    template <typename... Args>
    void initialize(Args&&... args) {
        new (ptr()) T(forward<Args>(args)...);
        _active = true;
    }
};

} // namespace avr
