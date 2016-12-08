#pragma once

#include <stdint.h>

namespace dimg {

enum class PixelFormat : uint8_t {
    R8Unorm = 0,
    RGB8Unorm,
    RGBA8Unorm,

    R8Uint,

    R32Float,
    RGB32Float,
    RGBA32Float,

    Count,
};

struct Image {
    uint32_t    width{0};
    uint32_t    height{0};
    PixelFormat pixelFormat{PixelFormat::R8Unorm};
    void*       data{nullptr};

    ~Image();
};

// Set width and height in outData
bool ResizeImage(const Image& inData, Image& outData);
bool LoadImageFromFile(const char* fpath, Image* image);
bool WriteImageToFile(const char* fpath, uint32_t width, uint32_t height, PixelFormat format, void* data);
}
