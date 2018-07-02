#pragma once
#pragma comment(lib, "d3d12")

#include "RenderDevice.h"
#include "SimpleShaderLibrary.h"
#include "ResourceManager.h"
#include "Pool.h"
#include "DMath.h"
#include "ByteBuffer.h"

#include <memory>
#include <cstring>
#include <unordered_map>
#include <map>
#include <array>

#include <wrl.h>

#ifdef _DEBUG
#define DEBUG_DX12
#endif

#include <dxgi1_4.h>
#include <d3d12.h>

namespace gfx {
    using namespace Microsoft::WRL;

    struct BufferDX12 {
        ComPtr<ID3D12Resource> buffer;
        BufferAccessFlags accessFlags;
        BufferUsageFlags usageFlags;
    };

    struct PipelineStateDX12 {
        ComPtr<ID3D12PipelineState> pipelineState;
        PrimitiveType primitiveType;
    };

    struct ShaderDX12 {
        ComPtr<ID3DBlob> blob;
        ShaderType shaderType;
    };

    struct InputLayoutDX12 {
        std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
        uint32_t stride;
    };

    struct TextureDX12 {
        ComPtr<ID3D12Resource> resource;
    };


    class DX12Device : public RenderDevice {
    private:

        static const uint32_t FrameCount = 2;

        uint32_t m_winWidth, m_winHeight;
        bool m_usePrebuiltShaders;

        ComPtr<IDXGIFactory4> m_factory;
        ComPtr<ID3D12Device> m_dev;
        ComPtr<IDXGISwapChain3> m_swapchain;

        ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
        ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
        uint64_t m_fenceValues[FrameCount];

        ComPtr<ID3D12DescriptorHeap> m_srvDescriptorHeap;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12RootSignature> m_rootSignature;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        uint32_t m_rtvDescriptorSize;

        std::unordered_map<PipelineStateId, PipelineStateDX12> m_pipelinestates;
        std::unordered_map<BufferId, BufferDX12> m_buffers;
        std::unordered_map<ShaderId, ShaderDX12> m_shaders;
        std::unordered_map<VertexLayoutId, InputLayoutDX12> m_inputLayouts;
        std::unordered_map<TextureId, TextureDX12> m_textures;

        uint32_t m_frameIndex;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;

        std::vector<CommandBuffer*> m_submittedBuffers;

        SimpleShaderLibrary m_shaderLibrary;

        ByteBuffer m_drawItemByteBuffer;

    public:
        DX12Device() {};
        ~DX12Device();

        RenderDeviceApi GetDeviceApi() { return RenderDeviceApi::D3D12; };

        int32_t InitializeDevice(const DeviceInitialization& deviceInit);
        void ResizeWindow(uint32_t width, uint32_t height) {};
        void PrintDisplayAdapterInfo();

        BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData);

        ShaderId GetShader(ShaderType type, const std::string& functionName);
        void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData);

        PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
        TextureId CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* data);
        TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height,
            uint32_t depth) { return 0; }

        TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data) { return 0; }
        VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc);

        CommandBuffer* CreateCommandBuffer() { return 0; }
        void Submit(const std::vector<CommandBuffer*>& cmdBuffers) {}

        uint8_t* MapMemory(BufferId buffer, BufferAccess);
        void UnmapMemory(BufferId buffer);

        void RenderFrame();

        void DestroyResource(ResourceId resourceId) {}
    private:
        ShaderId CreateShader(ShaderType type, const std::string& source);
        ComPtr<ID3DBlob> CompileShader(ShaderType shaderType, const std::string& source);

        D3D12_RASTERIZER_DESC CreateRasterState(const RasterState& state);
        D3D12_BLEND_DESC CreateBlendState(const BlendState& state);
        D3D12_DEPTH_STENCIL_DESC CreateDepthState(const DepthState& state);

        // todo: make this stuff shared between dx
        void* TextureDataConverter(const D3D12_RESOURCE_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef);
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

        template<class T, class K>
        K UseHandleEmplaceConstRef(std::unordered_map<K, T>& map, K handle, const T& item) {
            map.emplace(handle, item);
            return handle;
        }

        template<gfx::ResourceType t, class K, class T>
        K GenerateHandleEmplaceConstRef(std::unordered_map<K, T>& map, const T& item) {
            K handle = GenerateHandle(t);
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