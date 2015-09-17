#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DXGI.h>

#include "gfx/RenderDevice.h"
#include "gfx/AttribLayout.h"
#include "gfx/ParamType.h"

#include "DX11InputLayoutCache.h"

namespace graphics {
    // Cheating here with this
    namespace dx11 {
        class CBufferDescriptor;
    }
    using namespace graphics::dx11;

    static D3D11_USAGE BufferUsageDX11[(uint32_t)BufferUsage::COUNT] = {
        D3D11_USAGE_IMMUTABLE, // Static
        D3D11_USAGE_DEFAULT,   // Dynamic -- Default instead?
    };

    static DXGI_FORMAT TextureFormatDX11[(uint32_t)TextureFormat::COUNT] = {
        DXGI_FORMAT_R32_FLOAT,          // R32F
        DXGI_FORMAT_R32G32B32_FLOAT,    // RGB32F
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGBAF
    };

    static DXGI_FORMAT DataFormatDX11[(uint32_t)DataFormat::COUNT] = {
        DXGI_FORMAT_R32_FLOAT,          // Red
        DXGI_FORMAT_R32G32B32_FLOAT,    // RGB
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGBA
    };
    
    static D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeDX11[(uint32_t)PrimitiveType::COUNT] = {
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,               // triangles!
        D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,  // patches_4?
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
    
    // For shader cb optimizations, 'constantBuffer' can be an array here
    struct ConstantBufferDX11 {
        std::vector<ID3D11Buffer*> constantBuffers;
        std::vector<CBufferDescriptor*> cBufferDescs;
    };
    
    //K this is best i can do for now
    // depending on type, only certain interface is populated
    struct ShaderDX11 {
        ShaderType shaderType;
        ID3D11VertexShader *vertexShader;
        ID3D11PixelShader *pixelShader;
        ConstantBufferHandle cbHandle;
        uint32_t inputLayoutHandle; // vertex only?
        uint32_t samplers; // not implemented yet
    };
    
    struct TextureDX11 {
        ID3D11Texture2D* texture; // ID3D11Resource instead? 
        ID3D11ShaderResourceView* shaderResourceView; 
        DXGI_FORMAT format;
    };

    struct InputLayoutDX11 {
        ID3D11InputLayout* inputLayout;
    };

    struct SamplerDX11 {
        ID3D11SamplerState* sampler;
    };

    using namespace Microsoft::WRL;
    class RenderDeviceDX11 : public RenderDevice {
    private:
        std::unordered_map<uint32_t, IndexBufferDX11> m_indexBuffers;
        std::unordered_map<uint32_t, VertexBufferDX11> m_vertexBuffers;
        std::unordered_map<uint32_t, ConstantBufferDX11> m_constantBuffers;
        std::unordered_map<uint32_t, ShaderDX11> m_shaders;
        // todo: put cache in for inputlayouts;
        std::unordered_map<uint32_t, InputLayoutDX11> m_inputLayouts;
        std::unordered_map<uint32_t, TextureDX11> m_textures;
        std::unordered_map<uint32_t, SamplerDX11> m_samplers;

        HWND m_hwnd;
        ComPtr<ID3D11Device> m_dev;
        ComPtr<ID3D11DeviceContext> m_devcon;
        ComPtr<IDXGISwapChain> m_swapchain;
        ComPtr<IDXGIFactory> m_factory;
        ComPtr<ID3D11RenderTargetView> renderTarget;

        DX11InputLayoutCache inputLayoutCache;

        // todo: rip this out and merge things...
        struct DX11State {
            IndexBufferHandle indexBufferHandle;
            IndexBufferDX11 *indexBuffer;

            ShaderHandle vertexShaderHandle;
            ShaderDX11 *vertexShader;

            ConstantBufferDX11* vsCBuffer;
            std::vector<uint32_t> vsCBufferDirtySlots;

            std::unordered_map<uint32_t, ID3D11ShaderResourceView*> vsTextures;
            std::vector<uint32_t> vsDirtyTextureSlots;

            ShaderHandle pixelShaderHandle;
            ShaderDX11 *pixelShader;

            InputLayoutCacheHandle inputLayoutHandle;
            InputLayoutDX11 *inputLayout;

            VertexBufferHandle vertexBufferHandle;
            VertexBufferDX11* vertexBuffer;

            D3D11_PRIMITIVE_TOPOLOGY primitiveType;

            DX11State() {
                vertexShaderHandle = 0;
                vertexShader = 0;
                pixelShaderHandle = 0;
                pixelShader = 0;
                inputLayoutHandle = 0;
                inputLayout = 0;
                primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        };

        DX11State m_currentState, m_pendingState;

    public:
        RenderDeviceDX11() {};
        int                     InitializeDevice(void *windowHandle, uint32_t windowHeight, uint32_t windowWidth);

        IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage);
        void                    DestroyIndexBuffer(IndexBufferHandle handle);
        
        VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage);
        void                    DestroyVertexBuffer(VertexBufferHandle handle);
        
        ShaderHandle            CreateShader(ShaderType shaderType, const char **source);
        void                    DestroyShader(ShaderHandle handle);
     
        TextureHandle           CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTexture2D(TextureFormat tex_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth);
        TextureHandle           CreateTextureCube(TextureFormat tex_format, uint32_t width, uint32_t height, void** data);

        void                    DestroyTexture(TextureHandle handle);

        void                    SwapBuffers();

        void                    PrintDisplayAdapterInfo();

        // Commands
        void UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t height, void* data);
        void UpdateTexture(TextureHandle handle, void* data, size_t size);

        void SetRasterizerState(uint32_t state);
        void SetDepthState(uint32_t state);
        void SetBlendState(uint32_t state);

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

        //hack for now 
        int defaultSamplerHandle;
        SamplerHandle CreateSampler();
        void SetSampler(SamplerHandle samplerHandle, ShaderHandle shaderHandle, uint32_t location);
        void DestroySampler(SamplerHandle handle);

        uint32_t CreateInputLayout(ID3DBlob *Shader);
        void SetInputLayout(uint32_t inputLayoutHandle);
        void DestroyInputLayout(uint32_t handle);

        uint32_t CreateConstantBuffer(ID3DBlob *vertexShader);
        void UpdateConstantBuffer(ConstantBufferDX11* cb, std::vector<uint32_t> dirtySlots);
        void DestroyConstantBuffer(ConstantBufferHandle handle);

        inline uint32_t GenerateHandle() {
            static uint32_t key = 0;
            return ++key;
        }

        template <class T> T* Get(std::unordered_map<uint32_t, T> &map, uint32_t handle) {
            auto it = map.find(handle);
            if(it == map.end()) {
                return nullptr;
            }
            return &(*it).second;
        }
    };
}