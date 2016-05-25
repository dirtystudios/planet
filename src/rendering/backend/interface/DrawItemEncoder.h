#pragma once

#include "DrawItemDesc.h"
#include "DrawItem.h"
#include <stdint.h>
#include <vector>
#include "ResourceTypes.h"
#include "Bytebuffer.h"
#include <cassert>

namespace graphics {
class DrawItemEncoder {    
private:
    ByteBuffer* _byteBuffer;
public:
    DrawItemEncoder(ByteBuffer* byteBuffer) : _byteBuffer(byteBuffer) {};
    const DrawItem* Encode(const DrawItemDesc& desc);
};
}