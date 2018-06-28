#pragma once
#include "RenderDevice.h"
#include "DX11Context.h"
#include "SimpleShaderLibrary.h"
#include "DX11Resources.h"
#include "ResourceManager.h"
#include "ResourceTypes.h"
#include "DX11Cache.h"

#include <memory>
#include <cstring>
#include <unordered_map>
#include <map>
#include <array>

#ifdef _DEBUG
#define DEBUG_DX11
#endif

#ifdef DX11_3_API
#include <d3d11_3.h>
#else
#include <d3d11.h>
#endif

namespace gfx {
    class DX11Device final: public RenderDevice {
    private:
        bool m_usePrebuiltShaders;
#ifdef DX11_3_API
        Microsoft::WRL::ComPtr<ID3D11Device3> m_dev;
#else
        Microsoft::WRL::ComPtr<ID3D11Device> m_dev;
#endif
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;

        DX11Cache m_cache;
        std::unique_ptr<DX11Context> m_immediateContext;

        ResourceManager* m_resourceManager{ nullptr };
        SimpleShaderLibrary m_shaderLibrary;

    public:
        DX11Device() = delete;
        DX11Device(ResourceManager* resourceManager, bool usePrebuiltShaders=false);

        // DX11 Specific 
#ifdef DX11_3_API
        ID3D11Device3* GetID3D11Dev() { return m_dev.Get();  }
#else
        ID3D11Device* GetID3D11Dev() { return m_dev.Get(); }
#endif
        // Overrides

        RenderDeviceApi GetDeviceApi() final { return RenderDeviceApi::D3D11; };

        BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData) final;

        ShaderId GetShader(ShaderType type, const std::string& functionName) final;
        void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) final;

        PipelineStateId CreatePipelineState(const PipelineStateDesc& desc) final;

        TextureId CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName = "") final;
        TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height,
            uint32_t depth, const std::string& debugName) final;

        TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName) final;
        VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc) final;

        RenderPassId CreateRenderPass(const RenderPassInfo& renderPassInfo) final;
        CommandBuffer* CreateCommandBuffer() final;
        void UpdateTexture(TextureId textureId, uint32_t slice, const void* srcData) final;
        void Submit(const std::vector<CommandBuffer*>& cmdBuffers) final;

        uint8_t* MapMemory(BufferId buffer, BufferAccess) final;
        void UnmapMemory(BufferId buffer) final;

        void DestroyResource(ResourceId resourceId) final {};
    private:
        ID3D11DepthStencilState* CreateDepthState(const DepthState& state);
        ID3D11RasterizerState* CreateRasterState(const RasterState& state);
        ID3D11BlendState* CreateBlendState(const BlendState& state);

        ShaderId CreateShader(ShaderType type, const std::string& source, const std::string& name);

        // Texture Converter.
        // Returns pointer to use for data, may point to data or unique_ptr, 
        // unique_ptr is used to clear allocated data if needed
        void* TextureDataConverter(const D3D11_TEXTURE2D_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef);

        void CreateDefaultSampler();
        Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(ShaderType shaderType, const std::string& source);

        int GetFormatByteSize(DXGI_FORMAT dxFormat) {
            switch (dxFormat) {
            case DXGI_FORMAT_R8_UINT: return 1;
            case DXGI_FORMAT_R8_UNORM: return 1;
            case DXGI_FORMAT_R32_FLOAT: return 4;
            case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
            case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
            default: return 0;
            }
        }
    };
}
