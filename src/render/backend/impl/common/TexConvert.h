#include <cstdint>

namespace gfx {
    // *sigh* need this, apparently amd and nividia probly do this in the driver anyway, 
    //      but not for directx
    //--------------------------------
    //  Attempts detecting and using ssse3 instructions to 'expand' to dst, alpha channel contains probably 0
    //      src and dst needs to be aligned to 16 bytes.
    //  If no ssse3 support or unaligned, slow path it is
    //--------------------------------
    void Convert24BitTo32Bit(const std::uintptr_t src, std::uintptr_t dst, uint32_t numPixels);
}