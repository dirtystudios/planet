#include "TexConvert.h"
#include "Log.h"

#include <simde/x86/sse.h>
#include <simde/x86/sse2.h>
#include <simde/x86/ssse3.h>

namespace gfx {
    void Convert24BitTo32Bit_Generic(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels) {
        uint32_t* src32 = (uint32_t*)src;
        uint32_t* dst32 = (uint32_t*)dst;
        for (uint32_t i = 0; i < numPixels; i += 4) {
            uint32_t sa = src32[0];
            uint32_t sb = src32[1];
            uint32_t sc = src32[2];

            dst32[i + 0] = sa;
            dst32[i + 1] = (sa >> 24) | (sb << 8);
            dst32[i + 2] = (sb >> 16) | (sc << 16);
            dst32[i + 3] = sc >> 8;

            src32 += 3;
        }
    }

    void Convert24BitTo32Bit(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels) {
        auto src128 = (const simde__m128i*)src;
        auto dst128 = (simde__m128i*)dst;
        auto mask = simde_mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);

        for (uint32_t i = 0; i < numPixels; i += 16) {
            auto sa = simde_mm_loadu_si128(src128);
            auto sb = simde_mm_loadu_si128(src128 + 1);
            auto sc = simde_mm_loadu_si128(src128 + 2);

            auto val = simde_mm_shuffle_epi8(sa, mask);
            simde_mm_storeu_si128(dst128, val);
            val = simde_mm_shuffle_epi8(simde_mm_alignr_epi8(sb, sa, 12), mask);
            simde_mm_storeu_si128(dst128 + 1, val);
            val = simde_mm_shuffle_epi8(simde_mm_alignr_epi8(sc, sb, 8), mask);
            simde_mm_storeu_si128(dst128 + 2, val);
            val = simde_mm_shuffle_epi8(simde_mm_alignr_epi8(sc, sc, 4), mask);
            simde_mm_storeu_si128(dst128 + 3, val);

            src128 += 3;
            dst128 += 4;
        }
    } 
}
