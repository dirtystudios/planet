#include "TexConvert.h"
#include "Log.h"
#include "CpuId.h"

#include <emmintrin.h>
#include <tmmintrin.h>

namespace gfx {
    void Convert24BitTo32Bit(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels) {
        CpuId cpuId(1);
        bool hasSSSE3 = (cpuId.ECX() & ((int)1 << 9)) != 0;
        bool hasSSE41 = (cpuId.ECX() & ((int)1 << 19)) != 0;
        bool alignedSource = ((src & 15) == 0);
        bool alignedDst = ((dst & 15) == 0);

        if (alignedSource && alignedDst) {
            if (hasSSSE3) {
                __m128i *src128 = (__m128i*)src;
                __m128i *dst128 = (__m128i*)dst;
                __m128i mask = _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);

                for (uint32_t i = 0; i < numPixels; i += 16) {
                    __m128i sa = _mm_load_si128(src128);
                    __m128i sb = _mm_load_si128(src128 + 1);
                    __m128i sc = _mm_load_si128(src128 + 2);

                    __m128i val = _mm_shuffle_epi8(sa, mask);
                    _mm_store_si128(dst128, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sb, sa, 12), mask);
                    _mm_store_si128(dst128 + 1, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sb, 8), mask);
                    _mm_store_si128(dst128 + 2, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sc, 4), mask);
                    _mm_store_si128(dst128 + 3, val);

                    src128 += 3;
                    dst128 += 4;
                }
            }
            return;
        }
        LOG_D("Convert24bit: Slow Path. HasSSSE3: %s. HasSSE41: %s alignedSource: %d. alignedDst %d.",
            (hasSSSE3 ? "true" : "false"), (hasSSE41 ? "true" : "false"), alignedSource, alignedDst);
        uint32_t *src32 = (uint32_t*)src;
        uint32_t *dst32 = (uint32_t*)dst;

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
    }
