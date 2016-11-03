#pragma once

#include "Binding.h"
#include "RenderDevice.h"

// template <typename T>
// class MappedMemory {
// public:
//    using UnmapDelegate = std::function<void(T*)>;
// private:
//    UnmapDelegate _unmap;
//    T* _ptr{nullptr};
// public:
//    MappedMemory(void* ptr, UnmapDelegate unmap = UnmapDelegate()) : _ptr(reinterpret_cast<T*>(ptr)), _unmap(unmap) {
//        dg_assert_nm(ptr != nullptr);
//    }
//
//    ~MappedMemory() {
//        _unmap(_ptr);
//    }
//
//    T* operator->() {
//        return _ptr;
//    }
//
//
//    T& operator[](size_t idx) {
//        return _ptr[idx];
//    }
//};
//
// template <gfx::BufferAccessFlags Access, gfx::BufferUsageFlags Usage, gfx::BufferLifetime Lifetime>
// class Buffer {
// protected:
//    gfx::RenderDevice* _device{nullptr};
//    const gfx::BufferDesc desc;
//    const gfx::BufferId id{0};
// public:
//    Buffer(gfx::RenderDevice* device, const gfx::BufferDesc& desc, capacity, void* initialData = nullptr) : _device(device), desc({Usage, Access, Lifetime, false,
//    capacity}), id(_device->AllocateBuffer(desc, initialData)) {
//        dg_assert_nm(id != 0);
//    }
//
//    ~Buffer() {
//        _device->DestroyResource(id);
//    }
//};
//
// template <typename T>
// class BufferDataWriter {
// public:
//    BufferDataWriter(Buffer* buffer);
//
//    MappedMemory<T> write();
//};
//
// template <class >
// class BufferDataReader {
//
//};
//
// using VertexBuffer = Buffer<gfx::BufferAccessFlags::GpuReadCpuWriteBits, gfx::BufferUsageFlags::VertexBufferBit, gfx::BufferLifetime::Persistent>;
// using IndexBuffer = Buffer<gfx::BufferAccessFlags::GpuReadCpuWriteBits, gfx::BufferUsageFlags::IndexBufferBit, gfx::BufferLifetime::Persistent>;
// using ConstantBuffer = Buffer<gfx::BufferAccessFlags::GpuReadCpuWriteBits, gfx::BufferUsageFlags::ConstantBufferBit, gfx::BufferLifetime::Persistent>;
// using TransientConstantBuffer = Buffer<gfx::BufferAccessFlags::GpuReadCpuWriteBits, gfx::BufferUsageFlags::ConstantBufferBit, gfx::BufferLifetime::Transient>;

class ConstantBuffer {
private:
    gfx::BufferId      _constantBuffer;
    gfx::RenderDevice* _device;
    uint8_t*           _mappedPtr = nullptr;
    gfx::Binding       _binding;

public:
    ConstantBuffer(gfx::BufferId buffer, gfx::RenderDevice* device) : _constantBuffer(buffer), _device(device) {
        _binding.resource   = _constantBuffer;
        _binding.type       = gfx::Binding::Type::ConstantBuffer;
        _binding.stageFlags = gfx::ShaderStageFlags::AllStages;
    }

    template <class T>
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
