# avr-utils

A collection of C++ utilities for AVR programming.

AVR specific hardware abstractions:
- **I2C** master and slave
- **UART**
- **Tiner** management
- **Pin** and **PinGroup** typesafe abstractions over raw port bit operations

Drivers for:
- **Shift registers**
- I2C RTC **DS1307**

General utilities:
- **Circular buffer**
- **Tuple**
- **Variant**
- Metaprogramming utilities like `enable_if`, `remove_cv`, `forward`, ...
- **Serializable**: a *very very* simple template based serialization library

## Example CMakeLists.txt

To include `avr-utils` in your CMake-based project, use this in your `CMakeLists.txt`:

```cmake
cmake_minimum_required (VERSION 3.1)

# Target microcontroller info
set (AVR_MCU              atmega88pa)
set (AVR_MCU_SPEED        8000000UL)
set (AVR_MCU_FUSES_LFUSE  0xE2)
set (AVR_MCU_FUSES_HFUSE  0xD9)
set (AVR_MCU_FUSES_EFUSE  0xFD)
set (AVR_UPLOADTOOL       avrdude)
set (AVR_UPLOADTOOL_PORT  /dev/ttyACM0)
set (AVR_UPLOAD_SPEED     19200)
set (AVR_PROGRAMMER       stk500v1)

# Load the avr-utils. Change this to the correct path to avr-utils.
add_subdirectory (lib/avr-utils)

# The main output
project (MyProject)
add_avr_executable (MyProject src/main.cpp)
avr_target_link_libraries (MyProject avr-utils)
```