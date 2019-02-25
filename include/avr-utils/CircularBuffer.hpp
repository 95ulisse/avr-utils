#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "avr-utils/utility.hpp"

namespace avr {

namespace detail {

template <size_t N>
struct CircularBufferStorage {
    unsigned char data[N];
};

template <>
struct CircularBufferStorage<0> {
    unsigned char* data;
};

} // namespace detail




/**
 * Circular ring buffer suitable for usage from interrupt handlers with statically allocated buffer.
 * To support dynamic allocation, use CircularBuffer<0>.
 */
template <size_t N>
class CircularBuffer {
public:

    static_assert(N == 0 || N >= 2, "Invalid buffer size.");

    template <typename Dummy = void, typename = enable_if_t<N == 0, Dummy>>
    CircularBuffer(size_t capacity)
        : _capacity(capacity)
    {
        if (capacity < 2) {
            abort();
        }

        _buf.data = (char*) malloc(capacity);
        if (_buf == nullptr) {
            abort();
        }
    }

    template <typename Dummy = void, typename = enable_if_t<N != 0, Dummy>>
    CircularBuffer()
        : _capacity(N)
    {
    }

    inline size_t capacity() const {
        return _capacity;
    }

    inline size_t available() const {
        size_t ret;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            ret = _available();
        }
        return ret;
    }

    inline bool isEmpty() const {
        bool ret;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            ret = _isEmpty();
        }
        return ret;
    }

    inline bool isFull() const {
        bool ret;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            ret = _isFull();
        }
        return ret;
    }

    inline CircularBuffer& clear() {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            _end = _start = 0;
            _full = false;
        }
        return *this;
    }

    inline CircularBuffer& write(uint8_t x) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { _write(x); }
        return *this;
    }

    inline uint8_t read() {
        uint8_t ret;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            ret = _read();
        }
        return ret;
    }

private:
    const size_t _capacity;
    volatile detail::CircularBufferStorage<N> _buf;
    volatile size_t _start = 0;
    volatile size_t _end = 0;
    volatile bool _full = false;

    inline size_t _available() const {
        size_t s = _start, e = _end;
        bool f = _full;

        if (e == s && f) {
            return _capacity;
        }

        if (s <= e) {
            return e - s;
        } else {
            return _capacity + e - s;
        }
    }

    inline bool _isEmpty() const {
        return _end == _start && !_full;
    }

    inline bool _isFull() const {
        return _end == _start && _full;
    }

    inline void _write(uint8_t value) {

        // Be sure that there's space to write the value
        if (_isFull()) {
            abort();
        }

        // Copy the new value into the buffer
        _buf.data[_end] = value;
        _end = (_end + 1) % _capacity;
        _full = _end == _start;

    }

    inline uint8_t _read() {

        // Be sure that there's actually something to read
        if (_isEmpty()) {
            abort();
        }

        // Copy the byte from the buffer to the temporary
        uint8_t value = _buf.data[_start];
        _start = (_start + 1) % _capacity;
        _full = false;

        return value;

    }

};

} // namespace avr