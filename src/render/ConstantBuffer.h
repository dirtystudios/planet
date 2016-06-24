#pragma once

#include "RenderDevice.h"
#include "Binding.h"
class ConstantBuffer {
private:
    gfx::BufferId _constantBuffer;
    gfx::RenderDevice* _device;
    uint8_t* _mappedPtr = nullptr;
    gfx::Binding _binding;
public:
    ConstantBuffer(gfx::BufferId buffer, gfx::RenderDevice* device) : _constantBuffer(buffer), _device(device) {
        _binding.resource = _constantBuffer;
        _binding.type = gfx::Binding::Type::ConstantBuffer;
    }
    

    template<class T>
    T* Map(gfx::BufferAccess access = gfx::BufferAccess::Write) {
        return reinterpret_cast<T*>(Map(access));
    }

    uint8_t* Map(gfx::BufferAccess access = gfx::BufferAccess::Write) {
        assert(_mappedPtr == nullptr);
        _mappedPtr = _device->MapMemory(_constantBuffer, access);
        assert(_mappedPtr);
        return _mappedPtr;
    }

    void Unmap() {
        assert(_mappedPtr || "Someone forgot to unmap");
        _device->UnmapMemory(_constantBuffer);
        _mappedPtr = nullptr;
    }

    const gfx::Binding& GetBinding(uint32_t slot) {
        _binding.slot = slot;
        return _binding;
    }
};

template <class T>
class TypedConstantBuffer {
private:
    ConstantBuffer* _cbuffer;
public:
    TypedConstantBuffer(ConstantBuffer* constantBuffer) {
        
    }
    
    T* Map() {
        return reinterpret_cast<T>(_cbuffer->Map());
    }
    
    void Unmap() {
        _cbuffer->Unmap();
    }
    
    ConstantBuffer* constantBuffer() {
        return _cbuffer;
    }
    
};
