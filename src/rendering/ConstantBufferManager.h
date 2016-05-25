#pragma once

#include "RenderDevice.h"
#include "ConstantBuffer.h"

class ConstantBufferManager {
private:
    graphics::RenderDevice* _device;
    std::vector<ConstantBuffer*> _buffers;
public:
    ConstantBufferManager(graphics::RenderDevice* device) : _device(device) {};
    
    ConstantBuffer* GetConstantBuffer(size_t len) {
        graphics::BufferId buffer = _device->AllocateBuffer(graphics::BufferType::ConstantBuffer, len, graphics::BufferUsage::Dynamic);
        ConstantBuffer* cbuffer = new ConstantBuffer(buffer, _device);
        _buffers.push_back(cbuffer);
        return cbuffer;
    }
    
    template <class T>
    TypedConstantBuffer<T> GetConstantBuffer() {
        ConstantBuffer* cb = GetConstantBuffer(sizeof(T));
        assert(cb);
        return TypedConstantBuffer<T>(cb);
    }
};