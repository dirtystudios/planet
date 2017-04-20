#pragma once

#include "BufferLifetime.h"
#include "BufferUsageFlags.h"
#include "BufferAccessFlags.h"
#include <string>


namespace gfx {

struct BufferDesc {
    BufferUsageFlags usageFlags{BufferUsageFlags::None};                   // in what ways will the buffer be used
    BufferAccessFlags accessFlags{BufferAccessFlags::GpuReadCpuWriteBits}; // how will this buffer be accessed
    BufferLifetime lifetime{BufferLifetime::Persistent};
    bool isDynamic{false}; // will you be writing to the buffer while the gpu is using it
    size_t size{0};
    std::string debugName{""};

    static BufferDesc defaultTransient(BufferUsageFlags usageFlags, size_t size, const std::string& debugName = "") {
        BufferDesc bd;
        bd.usageFlags  = usageFlags;
        bd.accessFlags = BufferAccessFlags::GpuReadCpuWriteBits;
        bd.lifetime    = BufferLifetime::Transient;
        bd.isDynamic   = false;
        bd.size        = size;
        bd.debugName   = debugName;
        return bd;
    }

    static BufferDesc vbPersistent(size_t size, const std::string& debugName = "") { return defaultPersistent(BufferUsageFlags::VertexBufferBit, size); }

    static BufferDesc ibPersistent(size_t size, const std::string& debugName = "") { return defaultPersistent(BufferUsageFlags::IndexBufferBit, size); }

    static BufferDesc defaultPersistent(BufferUsageFlags usageFlags, size_t size, const std::string& debugName = "" ) {
        BufferDesc bd;
        bd.usageFlags  = usageFlags;
        bd.accessFlags = BufferAccessFlags::GpuReadCpuWriteBits;
        bd.lifetime    = BufferLifetime::Persistent;
        bd.isDynamic   = false;
        bd.size        = size;
        bd.debugName   = debugName;
        return bd;
    }
};
}
