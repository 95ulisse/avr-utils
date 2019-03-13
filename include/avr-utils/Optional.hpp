#pragma once

#include "avr-utils/utility.hpp"

namespace avr {

/** Inactive state of an optional. */
struct nullopt_t {};
static constexpr nullopt_t nullopt = {};



/** An optional represents a value that might be present or not. */
template <typename T>
class Optional {
public:

    // Initialization constructors

    Optional() : Optional(nullopt) {}

    Optional(nullopt_t) {}

    Optional(const T& x) { initialize(x); }

    Optional(T&& x) { initialize(move(x)); }

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

    // Observers

    inline operator bool()  const { return _active; }
    inline bool has_value() const { return _active; }

    inline const T& value() const& { if (!_active) abort(); else return contained_obj(); }
    inline T&       value() &      { if (!_active) abort(); else return contained_obj(); }
    inline T&&      value() &&     { if (!_active) abort(); else return move(contained_obj()); };

    inline T* operator->() { return ptr(); }

    inline const T& operator*() const& { return contained_obj(); }
    inline T&       operator*() &      { return contained_obj(); }
    inline T&&      operator*() &&     { return move(contained_obj()); }
    
    template <typename V>
    inline T value_or(V&& v) const& {
        return _active ? contained_obj() : static_cast<T>(v);
    }

    template <typename V>
    inline T value_or(V&& v) && {
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

    inline T* ptr() {
        return reinterpret_cast<T*>(&_storage.data);
    }

    inline T& contained_obj() &   { return *ptr(); }
    inline T&& contained_obj() && { return move(*ptr()); }

    template <typename... Args>
    void initialize(Args&&... args) {
        new (ptr()) T(forward<Args>(args)...);
        _active = true;
    }
};

} // namespace avr
