#pragma once

#include <stdint.h>

enum PixelFormat { GREY, GREY_ALPHA, RGB, RGBA };

struct Image {
    int32_t     width;
    int32_t     height;
    PixelFormat pixel_format;
    uint8_t*    data;

    ~Image();
};

bool LoadImageFromFile(const char* fpath, Image* image);
