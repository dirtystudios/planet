#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Log.h"

Image::~Image() {
    if(data) {
        stbi_image_free(data);
    }
}

bool LoadImageFromFile(const char* fpath, Image* image) {
    int components = 0;
    uint8_t* data = stbi_load(fpath, &image->width, &image->height, &components, 0);
    if(!data) {
        LOG_D("Failed to load image %s", fpath);
        return false;
    }

    image->data = data;
    LOG_D("%s, w:%d h:%d comp:%d", fpath, image->width, image->height, components);
    switch(components) {
        case 1: {
            image->pixel_format = GREY;
            break;
        }
        case 2: {
            image->pixel_format = GREY_ALPHA;
            break;
        };
        case 3: {
            image->pixel_format = RGB;
            break;
        };
        case 4: {
            image->pixel_format = RGBA;
            break;
        };
    }
    return true;
}
