#pragma once

#include <stdlib.h>

// Placement new
inline void* operator new (size_t n, void* ptr) { return ptr; }