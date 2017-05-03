#pragma once


namespace memory {    
    constexpr static size_t kBytesPerKilobyte = 1024;

    constexpr inline size_t KilobytesToBytes(size_t kilobytes) {
        return kilobytes * kBytesPerKilobyte;
    }
}