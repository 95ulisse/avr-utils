#pragma once

#include <avr/eeprom.h>
#include <inttypes.h>

#include "avr-utils/utility.hpp"

namespace avr {
namespace detail {

/** Functions to load and store raw values from the eeprom. */
template <typename T>
struct EEPROMStorageAccessor {
    static inline void load(void* ramValue, const void* eepromPointer) {
        eeprom_read_block((void*) ramValue, eepromPointer, sizeof(T));
    }
    static inline void store(const void* ramValue, void* eepromPointer) {
        eeprom_update_block((const void*) ramValue, eepromPointer, sizeof(T));
    }
};

// Specializations for the integral types

#define AVR_UTILS_SPECIALIZE_ACCESSOR(T, F, U)                                 \
    template <>                                                                \
    struct EEPROMStorageAccessor<T> {                                          \
        static inline void load(void* ramValue, const void* eepromPointer) {   \
            *((T*) ramValue) = eeprom_read_ ## F((const U*) eepromPointer);    \
        }                                                                      \
        static inline void store(const void* ramValue, void* eepromPointer) {  \
            eeprom_update_ ## F((U*) eepromPointer, *((U*) ramValue));         \
        }                                                                      \
    };

AVR_UTILS_SPECIALIZE_ACCESSOR(uint8_t, byte, uint8_t)
AVR_UTILS_SPECIALIZE_ACCESSOR(int8_t, byte, uint8_t)
AVR_UTILS_SPECIALIZE_ACCESSOR(uint16_t, word, uint16_t)
AVR_UTILS_SPECIALIZE_ACCESSOR(int16_t, word, uint16_t)
AVR_UTILS_SPECIALIZE_ACCESSOR(uint32_t, dword, uint32_t)
AVR_UTILS_SPECIALIZE_ACCESSOR(int32_t, dword, uint32_t)

#undef AVR_UTILS_SPECIALIZE_ACCESSOR

    
    
/** Type-safe access to a value stored in the EEPROM. */
template <typename T, typename Tag>
class EEPROMStorage {
public:

    static_assert(is_trivially_copyable_v<T>, "The type to be stored in the EEPROM must be trivially copyable.");

    EEPROMStorage() {}

    inline EEPROMStorage& load() {
        EEPROMStorageAccessor<T>::load((void*) &cache.bytes, (const void*) &data);
        return *this;
    }
    
    inline const EEPROMStorage& store() const {
        EEPROMStorageAccessor<T>::store((const void*) &cache.bytes, (void*) &data);
        return *this;
    }

    inline T& operator*() {
        return cache.obj;
    }

    inline T* operator->() {
        return &cache.obj;
    }

    inline const T& operator=(const T& other) {
        cache.obj = other;
        return other;
    }

private:
    static T data;
    
    union {
        T obj;
        typename aligned_storage<sizeof(T), alignof(T)>::type bytes;
    } cache;
    
    EEPROMStorage(const EEPROMStorage&) = delete;
    EEPROMStorage(EEPROMStorage&&) = delete;
};

} // namespace detail
} // namespace avr



#define EEPROMStorage(type, name, initialValue)                                          \
    namespace avr {                                                                      \
    namespace detail {                                                                   \
        /* Tag structure to create different template instantiations */                  \
        /* for each variable to store in the EEPROM. */                                  \
        struct __eeprom__tag__ ## name;                                                  \
        using __eeprom__type__ ## name = EEPROMStorage<type, __eeprom__tag__ ## name>;   \
    }                                                                                    \
    }                                                                                    \
    avr::detail::__eeprom__type__ ## name name;                                          \
    template<> type avr::detail::__eeprom__type__ ## name ::data EEMEM = initialValue;
