#pragma once

#include "CommandBuffer.h"
#include <vector>
#include "Bytebuffer.h"

namespace gfx {
    
    class GLCommandBuffer : public CommandBuffer {
    public:
        enum class CommandType {
            Clear,
            DrawItem,
            BindResource
        };

    private:
        static constexpr int kBufferSize = 4096;
        ByteBuffer _byteBuffer;
    public:
        GLCommandBuffer() {
            _byteBuffer.Resize(kBufferSize);
            Reset();
        }
        ~GLCommandBuffer() {
        }
        
        void Clear(float r, float g, float b, float a) {
            _byteBuffer << CommandType::Clear;
        }
        
        void DrawItem(const struct DrawItem* drawItem) {        
            _byteBuffer << CommandType::DrawItem << drawItem;
        }
        
        void BindResource(const Binding& binding) {
            _byteBuffer << CommandType::BindResource << binding;
        }
        
        // TODO:: readonly interface for bytebuffer?
        ByteBuffer& GetByteBuffer() {
            return _byteBuffer;
        }
                
        void Reset() {
            _byteBuffer.Reset();
        }
    };
}

