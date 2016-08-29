#pragma once
#include "RenderDevice.h"
#include "DX11Context.h"
#include "CommandBuffer.h"
#include "SimpleShaderLibrary.h"
#include "Pool.h"
#include "DMath.h"
#include "ByteBuffer.h"
#include "ResourceManager.h"

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
    class DX11Device : public RenderDevice {
    private:

        uint32_t m_winWidth, m_winHeight;
        bool m_usePrebuiltShaders;
#ifdef DX11_3_API
        ComPtr<ID3D11Device3> m_dev;
        ComPtr<IDXGISwapChain1> m_swapchain;
        ComPtr<IDXGIFactory3> m_factory;
#else
        ComPtr<ID3D11Device> m_dev;
        ComPtr<IDXGISwapChain> m_swapchain;
        ComPtr<IDXGIFactory> m_factory;
#endif
        ComPtr<ID3D11RenderTargetView> m_renderTarget;
        ComPtr<ID3D11DepthStencilView> m_depthStencilView;
        ComPtr<ID3D11SamplerState> m_defaultSampler;

        DX11ContextState previousState, nextState;

        struct PipelineStateDX11 {
            size_t vertexShaderHandle{ 0 };
            size_t pixelShaderHandle{ 0 };
            size_t vertexLayoutHandle{ 0 };
            size_t blendStateHandle{ 0 };
            size_t rasterStateHandle{ 0 };
            size_t depthStateHandle{ 0 };

            ID3D11VertexShader* vertexShader{ 0 };
            ID3D11PixelShader* pixelShader{ 0 };
            ID3D11InputLayout* vertexLayout{ 0 };
            uint32_t vertexLayoutStride;
            D3D11_PRIMITIVE_TOPOLOGY topology;
            ID3D11BlendState* blendState;
            ID3D11RasterizerState* rasterState;
            ID3D11DepthStencilState* depthState;
        };

        struct BufferDX11 {
            ComPtr<ID3D11Buffer> buffer;
        };

        struct InputLayoutDX11 {
            ComPtr<ID3D11InputLayout> inputLayout;
            VertexLayoutDesc layoutDesc;
            uint32_t stride;
        };

        struct TextureDX11 {
            ComPtr<ID3D11Texture2D> texture; // ID3D11Resource instead? 
            ComPtr<ID3D11ShaderResourceView> shaderResourceView;
            DXGI_FORMAT format;
            PixelFormat requestedFormat;
        };

        struct ShaderDX11 {
            ComPtr<ID3DBlob> blob;
            ShaderType shaderType;
            ID3D11VertexShader *vertexShader;
            ID3D11PixelShader *pixelShader;
        };

        std::unordered_map<size_t, BufferDX11> m_buffers;
        std::unordered_map<size_t, ShaderDX11> m_shaders;
        std::unordered_map<size_t, PipelineStateDX11> m_pipelineStates;
        std::unordered_map<size_t, TextureDX11> m_textures;

        std::unordered_map<size_t, InputLayoutDX11> m_inputLayouts;
        std::unordered_map<size_t, ComPtr<ID3D11BlendState>> m_blendStates;
        std::unordered_map<size_t, ComPtr<ID3D11RasterizerState>> m_rasterStates;
        std::unordered_map<size_t, ComPtr<ID3D11DepthStencilState>> m_depthStates;

        std::unique_ptr<DX11Context> m_context;

        std::vector<CommandBuffer*> m_submittedBuffers;

        ResourceManager m_resourceManager;
        SimpleShaderLibrary m_shaderLibrary;
        Pool<CommandBuffer, 1> m_commandBufferPool;

        ByteBuffer m_drawItemByteBuffer;

    public:
        DX11Device() {};
        ~DX11Device();

        RenderDeviceApi GetDeviceApi() { return RenderDeviceApi::D3D11; };

        int32_t InitializeDevice(const DeviceInitialization& deviceInit);
        void ResizeWindow(uint32_t width, uint32_t height);
        void PrintDisplayAdapterInfo();

        BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData);

        ShaderId GetShader(ShaderType type, const std::string& functionName);
        void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData);

        PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
        TextureId CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* data);
        TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height,
            uint32_t depth);

        TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data);
        VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc);

        CommandBuffer* CreateCommandBuffer();
        void Submit(const std::vector<CommandBuffer*>& cmdBuffers);

        uint8_t* MapMemory(BufferId buffer, BufferAccess);
        void UnmapMemory(BufferId buffer);

        void RenderFrame();

        void DestroyResource(ResourceId resourceId) {};
    private:
        ID3D11DepthStencilState* CreateDepthState(const DepthState& state);
        ID3D11RasterizerState* CreateRasterState(const RasterState& state);
        ID3D11BlendState* CreateBlendState(const BlendState& state);

        ID3D11InputLayout* CreateInputLayout(InputLayoutDX11* state, ShaderId shaderId);

        ShaderId CreateShader(ShaderType type, const std::string& source);

        void SetPipelineState(PipelineStateDX11* state);
        void Execute(CommandBuffer* cmdBuffer);

        // Texture Converter.
        // Returns pointer to use for data, may point to data or unique_ptr, 
        // unique_ptr is used to clear allocated data if needed
        void* TextureDataConverter(const D3D11_TEXTURE2D_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef);

        void CreateSetDefaultSampler();
        ComPtr<ID3DBlob> CompileShader(ShaderType shaderType, const std::string& source);
        void ResetDepthStencilTexture();
        void ResetViewport();

        // todo: don't be eugene and actually delete these
        void DestroyBuffer(BufferId buffer) {}
        void DestroyShader(ShaderId shader) { 
            auto it = m_shaders.find(shader);
            if (it != m_shaders.end()) {
                it->second.pixelShader->Release();
                it->second.vertexShader->Release();
                m_shaders.erase(shader);
            }
        }
        void DestroyPipelineState(PipelineStateId pipelineState) {}
        void DestroyTexture(TextureId texture) {}
        void DestroyVertexLayout(VertexLayoutId vertexLayout) {}

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

        inline size_t GenerateHandle(gfx::ResourceType type) {
            static size_t key = 1;
            return key++;
        }

        template<gfx::ResourceType t, class T>
        size_t GenerateHandleEmplaceConstRef(std::unordered_map<size_t, T>& map, const T& item) {
            size_t handle = GenerateHandle(t);
            map.emplace(handle, item);
            return handle;
        }

        template<class K, class T>
        K UseHandleEmplaceConstRef(std::unordered_map<K, T>& map, K handle, const T& item) {
            map.emplace(handle, item);
            return handle;
        }

        template <class K, class T>
        T* GetResource(std::unordered_map<K, T>& map, K handle) {
            auto it = map.find(handle);
            if (it == map.end()) {
                return nullptr;
            }

            return &(*it).second;
        }
    };
}
