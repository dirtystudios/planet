#include "DrawItemEncoder.h"
#include <cassert>
namespace graphics {
const DrawItem* DrawItemEncoder::Encode(const DrawItemDesc& desc) {
    uint8_t* start = _byteBuffer->WritePos();
    
    (* _byteBuffer) << GetDrawItemSize(desc) << desc.drawCall << desc.pipelineState;
    (* _byteBuffer) << desc.bindingCount;
    _byteBuffer->Write(desc.bindings, desc.bindingCount);
    (* _byteBuffer) << desc.streamCount;
    _byteBuffer->Write(desc.streams, desc.streamCount);
    
    if (desc.drawCall.type == DrawCall::Type::Indexed) {
        (* _byteBuffer) << desc.indexBuffer << desc.offset;
    }
    
    return reinterpret_cast<const DrawItem*>(start);
}
}