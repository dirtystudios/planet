#pragma once

#include "RenderDevice.h"
#include "ConstantBuffer.h"

class ConstantBufferManager {
private:
    gfx::RenderDevice* _device;
    std::vector<ConstantBuffer*> _buffers;

public:
    ConstantBufferManager(gfx::RenderDevice* device) : _device(device){};

    ConstantBuffer* GetConstantBuffer(size_t len, const std::string& debugName = "") {
        gfx::BufferDesc desc    = 
            gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ConstantBufferBit, len, debugName + "CB");
        gfx::BufferId buffer    = _device->AllocateBuffer(desc);
        ConstantBuffer* cbuffer = new ConstantBuffer(buffer, _device);
        _buffers.push_back(cbuffer);
        return cbuffer;
    }

    template <class T>
    TypedConstantBuffer<T> GetConstantBuffer(const std::string& debugName = "") {
        ConstantBuffer* cb = GetConstantBuffer(sizeof(T), debugName);
        assert(cb);
        return TypedConstantBuffer<T>(cb);
    }
};
