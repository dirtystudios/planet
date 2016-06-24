#pragma once

#include "RenderDevice.h"
#include "ConstantBuffer.h"

class ConstantBufferManager {
private:
    gfx::RenderDevice* _device;
    std::vector<ConstantBuffer*> _buffers;
public:
    ConstantBufferManager(gfx::RenderDevice* device) : _device(device) {};
    
    ConstantBuffer* GetConstantBuffer(size_t len) {
        gfx::BufferId buffer = _device->AllocateBuffer(gfx::BufferType::ConstantBuffer, len, gfx::BufferUsage::Dynamic);
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
