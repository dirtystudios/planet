#pragma once

#include "RenderDevice.h"
#include "Binding.h"
class ConstantBuffer {
private:
    graphics::BufferId _constantBuffer;
    graphics::RenderDevice* _device;
    uint8_t* _mappedPtr = nullptr;
    graphics::Binding _binding;
public:
    ConstantBuffer(graphics::BufferId buffer, graphics::RenderDevice* device) : _constantBuffer(buffer), _device(device) {
        _binding.resource = _constantBuffer;
        _binding.type = graphics::Binding::Type::ConstantBuffer;
    }
    

    template<class T>
    T* Map(graphics::BufferAccess access = graphics::BufferAccess::Write) {
        return reinterpret_cast<T*>(Map(access));
    }

    uint8_t* Map(graphics::BufferAccess access = graphics::BufferAccess::Write) {
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

    const graphics::Binding& GetBinding(uint32_t slot) {
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
