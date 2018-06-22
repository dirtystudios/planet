#pragma once

#import <Metal/Metal.h>
#include "BufferDesc.h"
#include "PipelineStateDesc.h"
#include "PixelFormat.h"
#include "Resource.h"
#include "ShaderType.h"
#include "VertexLayoutDesc.h"
#include "RenderPassInfo.h"

namespace gfx {

    struct MetalBuffer : public Resource {
        ~MetalBuffer() { [mtlBuffer release]; }

        BufferDesc    desc;
        id<MTLBuffer> mtlBuffer;
    };

    struct MetalLibraryFunction : public Resource {
        ~MetalLibraryFunction() {
            [mtlLibrary release];
            [mtlFunction release];
        }

        id<MTLLibrary>  mtlLibrary;
        id<MTLFunction> mtlFunction;
        std::string     functionName;
        ShaderType      type;
    };

    struct MetalVertexLayout : public Resource {
        ~MetalVertexLayout() { [mtlVertexDesc release]; }

        VertexLayoutDesc     layoutDesc;
        MTLVertexDescriptor* mtlVertexDesc;
    };

    struct MetalPipelineState : public Resource {
        ~MetalPipelineState() {
            [mtlPipelineState release];
            [mtlDepthStencilState release];
            [reflection release];
        }

        id<MTLRenderPipelineState>   mtlPipelineState;
        id<MTLDepthStencilState>     mtlDepthStencilState;
        PipelineStateDesc            pipelineStateDesc;
        MTLRenderPipelineReflection* reflection;
    };

    struct MetalTexture : public Resource {
        ~MetalTexture() {
            [mtlTexture release];
            [mtlSamplerState release];
        }

        id<MTLTexture>      mtlTexture { nil };
        id<MTLSamplerState> mtlSamplerState { nil };
        PixelFormat         externalFormat { PixelFormat::Invalid };
        uint32_t            bytesPerRow { 0 };
        uint32_t            bytesPerImage { 0 };
    };
    
    struct MetalRenderPass : public Resource {
        RenderPassInfo info;
    };
}
