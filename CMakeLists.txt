cmake_minimum_required (VERSION 3.1)

# Include the toolchain
include (cmake/avr-gcc-toolchain.cmake)

# Compiler features
set (CMAKE_CXX_STANDARD 17)

# Source files
file (GLOB_RECURSE SOURCES "src/*.cpp" "src/**/*.cpp")

# Main library
project (avr-utils)
add_avr_library (avr-utils ${SOURCES})
target_include_directories (avr-utils PUBLIC include)