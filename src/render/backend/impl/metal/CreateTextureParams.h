//
//  CreateTextureParams.h
//  planet
//
//  Created by Eugene Sturm on 6/28/18.
//

#pragma once

#include <Metal/Metal.h>
#include "PixelFormat.h"
#include <string>

namespace gfx
{
    struct CreateTextureParams {
        std::string     debugName {""};
        PixelFormat     format{PixelFormat::R8Unorm};
        uint32_t        width{0};
        uint32_t        height{0};
        uint32_t        depth{1};
        uint32_t        mips{1};
        uint32_t        sampleCount{1};
        uint32_t        arrayLength{1};
        MTLCPUCacheMode cpuCacheMode{MTLCPUCacheModeDefaultCache};
        MTLStorageMode  storageMode{MTLStorageModeManaged};
        MTLTextureType  textureType{MTLTextureType2D};
        MTLTextureUsage usage{MTLTextureUsageShaderRead};
        void* const*    srcData{nullptr};
        uint32_t        srcDataCount{0};
    };
}
