#pragma once
#include <cstdint>

namespace gfx {
    // simd-ified
    void Convert24BitTo32Bit(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels);
    // generic slow loop
    void Convert24BitTo32Bit_Generic(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels);
}