#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "DGAssert.h"
#include "Log.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace dimg;

static uint32_t getComponents(PixelFormat format) {
    switch (format) {
        case PixelFormat::R8Unorm:
        case PixelFormat::R32Float:
            return 1;
        case PixelFormat::RGB8Unorm:
        case PixelFormat::RGB32Float:
            return 3;
        case PixelFormat::RGBA8Unorm:
        case PixelFormat::RGBA32Float:
            return 4;
        default:
            dg_assert_fail_nm();
    }
    return 0;
}

static PixelFormat getFormat(uint32_t components) {
    dg_assert_nm(components > 0 && components <= 4);
    switch (components) {
        case 1:
            return PixelFormat::R8Unorm;
        case 3:
            return PixelFormat::RGB8Unorm;
        case 4:
            return PixelFormat::RGBA8Unorm;
        default:
            dg_assert_fail_nm();
    }
    return PixelFormat::R8Unorm;
}

static uint32_t getPixelSize(PixelFormat format) {
    switch (format) {
        case PixelFormat::R8Unorm:
            return 1;
        case PixelFormat::R32Float:
            return 4;
        case PixelFormat::RGB8Unorm:
            return 3;
        case PixelFormat::RGB32Float:
            return 12;
        case PixelFormat::RGBA8Unorm:
            return 4;
        case PixelFormat::RGBA32Float:
            return 16;
        default:
            dg_assert_fail_nm();
    }
    return 0;
}

static uint32_t getStride(uint32_t width, PixelFormat format) { return width * getPixelSize(format); }

Image::~Image() {
    if (data) {
        stbi_image_free(data);
    }
}

bool dimg::WriteImageToFile(const char* fpath, uint32_t width, uint32_t height, PixelFormat format, void* data) {
    dg_assert_nm(format == PixelFormat::R8Unorm || format == PixelFormat::RGB8Unorm || format == PixelFormat::RGBA8Unorm);
    int result = stbi_write_png(fpath, width, height, getComponents(format), data, getStride(width, format));
    return result == 1;
}

bool dimg::LoadImageFromFile(const char* fpath, Image* image) {
    int      components = 0;
    uint8_t* data       = stbi_load(fpath, (int*)&image->width, (int*)&image->height, &components, 0);
    if (!data) {
        LOG_D("Failed to load image %s", fpath);
        return false;
    }

    image->data = data;
    LOG_D("%s, w:%d h:%d comp:%d", fpath, image->width, image->height, components);
    image->pixelFormat = getFormat(components);
    return true;
}
