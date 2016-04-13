#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DXGI.h>

#include "RenderDevice.h"
#include "AttribLayout.h"
#include "ParamType.h"

#include "DX11InputLayoutCache.h"

namespace graphics {
    // Cheating here with this
    namespace dx11 {
        class CBufferDescriptor;
    }
    using namespace graphics::dx11;

    static D3D11_USAGE BufferUsageDX11[(uint32_t)BufferUsage::COUNT] = {
        D3D11_USAGE_IMMUTABLE, // Static
        D3D11_USAGE_DYNAMIC,   // Dynamic
    };

    static DXGI_FORMAT TextureFormatDX11[(uint32_t)TextureFormat::COUNT] = {
        DXGI_FORMAT_R32_FLOAT,          // R32F
        DXGI_FORMAT_R32G32B32_FLOAT,    // RGB32F
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGBAF
        DXGI_FORMAT_R8_UNORM,           // Byte_UINT
        DXGI_FORMAT_R8G8B8A8_UNORM,     // RGB_UINT, Converted before load
    };
    
    static D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeDX11[(uint32_t)PrimitiveType::COUNT] = {
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,               // triangles!
        D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,  // patches_4?
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,                  // LineStrip
    };

    static D3D11_BLEND_OP BlendModeDX11[(uint32_t)BlendMode::COUNT] = {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX,
    };

    static D3D11_BLEND BlendFuncDX11[(uint32_t)BlendFunc::COUNT] {
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,
        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_INV_SRC_COLOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_DEST_ALPHA,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_INV_DEST_COLOR,
        D3D11_BLEND_SRC_ALPHA_SAT,
        D3D11_BLEND_SRC1_COLOR,
        D3D11_BLEND_INV_SRC1_COLOR,
        D3D11_BLEND_SRC1_ALPHA,
        D3D11_BLEND_INV_SRC1_ALPHA
    };

    static D3D11_FILL_MODE FillModeDX11[(uint32_t)FillMode::COUNT] {
        D3D11_FILL_WIREFRAME,
        D3D11_FILL_SOLID
    };

    static D3D11_CULL_MODE CullModeDX11[(uint32_t)CullMode::COUNT] {
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
    };

    static D3D11_DEPTH_WRITE_MASK DepthWriteMaskDX11[(uint32_t)DepthWriteMask::COUNT] {
        D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_DEPTH_WRITE_MASK_ALL
    };

    static D3D11_COMPARISON_FUNC DepthFuncDX11[(uint32_t)DepthFunc::COUNT] {
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS
    };

    typedef uint32_t SamplerHandle;
    typedef uint32_t ConstantBufferHandle;

    struct IndexBufferDX11 {
        ID3D11Buffer* indexBuffer = NULL;
    };

    struct VertexBufferDX11 {
        ID3D11Buffer* vertexBuffer = NULL;
        VertLayout layout;
    };
    
    struct ConstantBufferDX11 {
        std::vector<ID3D11Buffer*> constantBuffers;
        std::vector<CBufferDescriptor*> cBufferDescs;
    };
    
    // depending on type, only certain interface is populated
    struct ShaderDX11 {
        ShaderType shaderType;
        ID3D11VertexShader *vertexShader;
        ID3D11PixelShader *pixelShader;
        ConstantBufferHandle cbHandle;
        uint32_t inputLayoutHandle;
        //uint32_t samplers; // not implemented yet
    };
    
    struct TextureDX11 {
        ID3D11Texture2D* texture; // ID3D11Resource instead? 
        ID3D11ShaderResourceView* shaderResourceView; 
        DXGI_FORMAT format;
        TextureFormat requestedFormat;
    };

    struct InputLayoutDX11 {
        ID3D11InputLayout* inputLayout;
    };

    struct SamplerDX11 {
        ID3D11SamplerState* sampler;
    };

    struct BlendStateDX11 {
        ID3D11BlendState* blendState;
    };

    struct RasterStateDX11 {
        ID3D11RasterizerState* rasterState;
    };

    struct DepthStateDX11 {
        ID3D11DepthStencilState* depthState;
    };

    using namespace Microsoft::WRL;
    class RenderDeviceDX11 : public RenderDevice {
    private:
        uint32_t m_winWidth, m_winHeight;
        bool m_usePrebuiltShaders;
        std::unordered_map<uint32_t, IndexBufferDX11> m_indexBuffers;
        std::unordered_map<uint32_t, VertexBufferDX11> m_vertexBuffers;
        std::unordered_map<uint32_t, ConstantBufferDX11> m_constantBuffers;
        std::unordered_map<uint32_t, ShaderDX11> m_shaders;
        std::unordered_map<uint32_t, InputLayoutDX11> m_inputLayouts;
        std::unordered_map<uint32_t, TextureDX11> m_textures;
        std::unordered_map<uint32_t, SamplerDX11> m_samplers;
        std::unordered_map<uint32_t, BlendStateDX11> m_blendStates;
        std::unordered_map<uint32_t, RasterStateDX11> m_rasterStates;
        std::unordered_map<uint32_t, DepthStateDX11> m_depthStates;

        HWND m_hwnd;
        ComPtr<ID3D11Device> m_dev;
        ComPtr<ID3D11DeviceContext> m_devcon;
        ComPtr<IDXGISwapChain> m_swapchain;
        ComPtr<IDXGIFactory> m_factory;
        ComPtr<ID3D11RenderTargetView> m_renderTarget;
        ComPtr<ID3D11DepthStencilView> m_depthStencilView;

        DX11InputLayoutCache inputLayoutCache;

        // todo: rip this out and merge things...
        struct DX11State {
            IndexBufferHandle indexBufferHandle;
            IndexBufferDX11* indexBuffer;

            ShaderHandle vertexShaderHandle;
            ShaderDX11* vertexShader;

            ConstantBufferDX11* vsCBuffer;
            std::vector<uint32_t> vsCBufferDirtySlots;

            std::unordered_map<uint32_t, ID3D11ShaderResourceView*> vsTextures;
            std::vector<uint32_t> vsDirtyTextureSlots;

            ConstantBufferDX11* psCBuffer;
            std::vector<uint32_t> psCBufferDirtySlots;

            std::unordered_map<uint32_t, ID3D11ShaderResourceView*> psTextures;
            std::vector<uint32_t> psDirtyTextureSlots;

            ShaderHandle pixelShaderHandle;
            ShaderDX11* pixelShader;

            InputLayoutCacheHandle inputLayoutHandle;
            InputLayoutDX11* inputLayout;

            VertexBufferHandle vertexBufferHandle;
            VertexBufferDX11* vertexBuffer;

            int blendStateHash;
            BlendStateDX11* blendState;

            int rasterStateHash;
            RasterStateDX11* rasterState;

            int depthStateHash;
            DepthStateDX11* depthState;

            D3D11_PRIMITIVE_TOPOLOGY primitiveType;

            DX11State() {
                vertexShaderHandle = 0;
                vertexShader = 0;
                pixelShaderHandle = 0;
                pixelShader = 0;
                inputLayoutHandle = 0;
                inputLayout = 0;
                blendState = 0;
                blendStateHash = 0;
                rasterState = 0;
                rasterStateHash = 0;
                depthState = 0;
                depthStateHash = 0;
                primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        };

        DX11State m_currentState, m_pendingState;

    public:
        RenderDeviceDX11() {};
        ~RenderDeviceDX11();
        int                     InitializeDevice(DeviceInitialization deviceInitialization);

        IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage);
        void                    DestroyIndexBuffer(IndexBufferHandle handle);
        
        VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage);
        void                    DestroyVertexBuffer(VertexBufferHandle handle);
        
        ShaderHandle            CreateShader(ShaderType shaderType, const std::string& source);
        void                    DestroyShader(ShaderHandle handle);
     
        TextureHandle           CreateTexture2D(TextureFormat tex_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth);
        TextureHandle           CreateTextureCube(TextureFormat tex_format, uint32_t width, uint32_t height, void** data);

        void                    DestroyTexture(TextureHandle handle);

        void                    SwapBuffers();
        void                    ResizeWindow(uint32_t width, uint32_t height);
        void                    PrintDisplayAdapterInfo();

        // Commands
        void UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t height, void* data);
        void UpdateTexture(TextureHandle handle, void* data, size_t size);
        void UpdateVertexBuffer(VertexBufferHandle vertexBufferHandle, void* data, size_t size);

        void SetRasterState(const RasterState& rasterState);
        void SetDepthState(const DepthState& depthState);
        void SetBlendState(const BlendState& blendState);

        void Clear(float r, float g, float b, float a);
        void SetVertexShader(ShaderHandle shaderHandle);
        void SetPixelShader(ShaderHandle shaderHandle);
        void SetShaderParameter(ShaderHandle handle, ParamType paramType, const char *param_name, void *data);
        void SetShaderTexture(ShaderHandle shaderHandle, TextureHandle textureHandle, TextureSlot slot);
        void SetVertexBuffer(VertexBufferHandle handle);

        void DrawPrimitive(PrimitiveType primitiveType, uint32_t startVertex, uint32_t numVertices);

    private:
        //This goes here till renderDevice.h has it..too lazy
        void SetIndexBuffer(IndexBufferHandle handle);

        TextureHandle Texture2DCreator(D3D11_TEXTURE2D_DESC* tDesc, TextureFormat texFormat, D3D11_SHADER_RESOURCE_VIEW_DESC* viewDesc, void* data);

        // Resize Functions, 'Reset' means it recreates based on window size
        void ResetViewport();
        void ResetDepthStencilTexture();

        ComPtr<ID3DBlob> CompileShader(ShaderType shaderType, const std::string& source);

        //hack for now 
        int defaultSamplerHandle;
        SamplerHandle CreateSampler();
        void SetSampler(SamplerHandle samplerHandle, ShaderHandle shaderHandle, uint32_t location);
        void DestroySampler(SamplerHandle handle);

        uint32_t CreateInputLayout(ID3DBlob *Shader);
        uint32_t CreateInputLayout(uint32_t shaderHash, const std::string& byteCode);
        void SetInputLayout(uint32_t inputLayoutHandle);
        void DestroyInputLayout(uint32_t handle);

        uint32_t CreateConstantBuffer(ID3DBlob *shaderBlob);
        uint32_t CreateConstantBuffer(uint32_t shaderHash);
        void UpdateConstantBuffer(ConstantBufferDX11* cb, std::vector<uint32_t> dirtySlots);
        void DestroyConstantBuffer(ConstantBufferHandle handle);

        inline uint32_t GenerateHandle() {
            static uint32_t key = 0;
            return ++key;
        }

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

        template <class T> 
        T* Get(std::unordered_map<uint32_t, T> &map, uint32_t handle) {
            auto it = map.find(handle);
            if(it == map.end()) {
                return nullptr;
            }
            return &(*it).second;
        }

        template <class T, typename FType> 
        void ReleaseAllFromMap(std::unordered_map<uint32_t, T> &map, FType f) {
            for (auto it = map.begin(); it != map.end(); ++it) {
                if (it->second.*f)
                    (it->second.*f)->Release();
            }
        }
    };
}