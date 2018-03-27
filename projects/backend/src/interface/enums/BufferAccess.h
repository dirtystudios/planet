#pragma once 


namespace gfx {
enum class BufferAccess : uint8_t {
    Read = 0,
    Write,
    WriteNoOverwrite,
    ReadWrite,
    Count
};
}
