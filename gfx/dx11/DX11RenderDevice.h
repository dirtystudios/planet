#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <DXGI.h>


#include "../RenderDevice.h"
#include "../AttribLayout.h"
#include <unordered_map>

namespace graphics {

    static D3D11_USAGE bufferUsageDX11[(uint32_t)BufferUsage::COUNT] = {
        D3D11_USAGE_IMMUTABLE, // Static
        D3D11_USAGE_DEFAULT,   // Dynamic -- Default instead?
    };

    static DXGI_FORMAT textureFormatDX11[(uint32_t)TextureFormat::COUNT] = {
        DXGI_FORMAT_R32_FLOAT,          // R32F
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGB32F
    };

    static DXGI_FORMAT dataFormatDX11[(uint32_t)DataFormat::COUNT] = {
        DXGI_FORMAT_R32_FLOAT,          // Red
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RBG
    };
    
    struct IndexBufferDX11 {
        ID3D11Buffer* indexBuffer = NULL;
    };

    
    struct VertexBufferDX11 {
        ID3D11Buffer* vertexBuffer = NULL;
        VertLayout layout;
    };
    
    struct ShaderDX11 {
        ID3DBlob* shader;
        ShaderType type;
    };
    
    struct ProgramDX11 {
        ID3D11VertexShader *vertexShader;
        ID3D11PixelShader *pixelShader;
        ID3D11InputLayout *inputLayout;
        AttribLayout layout;
    };
    
    struct TextureDX11 {
        ID3D11Texture2D* texture; // ID3D11Resource instead? 
        ID3D11ShaderResourceView* shaderResourceView; // what do with this
        DXGI_FORMAT format;
    };
    
    struct ConstantBufferDX11 {
        ID3D11Buffer* constantBuffer;
        MemoryLayout layout;
    };

    struct SamplerStateDX11{
        ID3D11SamplerState* samplerState1;
        ID3D11SamplerState* samplerState2;
    };

    struct ShaderResourceViewDX11 {
        
    };
       
    using namespace Microsoft::WRL;
    class RenderDeviceDX11 : public RenderDevice {
    private:
        std::unordered_map<uint32_t, IndexBufferDX11> m_indexBuffers;
        std::unordered_map<uint32_t, VertexBufferDX11> m_vertexBuffers;
        std::unordered_map<uint32_t, ShaderDX11> m_shaders;
        std::unordered_map<uint32_t, ProgramDX11> m_programs;
        std::unordered_map<uint32_t, TextureDX11> m_textures;
        std::unordered_map<uint32_t, ConstantBufferDX11> m_constantBuffers;
        std::unordered_map<uint32_t, SamplerStateDX11> m_samplers;

        HWND m_hwnd;
        ComPtr<ID3D11Device> m_dev;
        ComPtr<ID3D11DeviceContext> m_devcon;
        ComPtr<IDXGISwapChain> m_swapchain;
        ComPtr<IDXGIFactory> m_factory;
        ComPtr<ID3D11RenderTargetView> renderTarget;

    public:
        int                     InitializeDevice(void* args);

        IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage);
        void                    DestroyIndexBuffer(IndexBufferHandle handle);
        
        VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage);
        void                    DestroyVertexBuffer(VertexBufferHandle handle);
        
        ShaderHandle            CreateShader(ShaderType shader_type, const char **source);
        void                    DestroyShader(ShaderHandle handle);
        
        ProgramHandle           CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders);
        void                    DestroyProgram(ProgramHandle handle);
        
        ConstantBufferHandle    CreateConstantBuffer(const MemoryLayout &layout, void *data, BufferUsage usage);
        void                    DestroyConstantBuffer(ConstantBufferHandle handle);

        TextureHandle           CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTexture2DArray(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, uint32_t depth, void* data) = 0;
        TextureHandle           CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data);
        void                    DestroyTexture(TextureHandle handle);

        SamplerHandle           CreateSamplers();
        void                    DestroySampler(SamplerHandle handle);

        void                    Clear(float* RGBA);
        void                    SwapBuffers();

        void                    PrintDisplayAdapterInfo();

        void UpdateConstantBuffer(ConstantBufferHandle handle, void* data, size_t size, size_t offset);
        void UpdateTexture(TextureHandle handle, void* data, size_t size);
        void UpdateTexture2DArray(TextureHandle handle, void* data, size_t size, uint32_t width, uint32_t height);
        void BindProgram(ProgramHandle handle);
        void SetRasterizerState(uint32_t state);
        void SetDepthState(uint32_t state);
        void SetBlendState(uint32_t state);
        void DrawArrays(VertexBufferHandle handle, uint32_t start_vertex, uint32_t num_vertices);
        void BindTexture(TextureHandle handle, uint32_t slot);
        void BindConstantBuffer(ConstantBufferHandle handle, uint32_t slot);  
        void SetProgramTexture(TextureHandle handle, const char *paramName, uint32_t slot);
        void BindSampler(SamplerHandle handle, uint32_t location);

    private:
        wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
        {
            wchar_t* wString = new wchar_t[4096];
            MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
            return wString;
        }

        template <class T> T* Get(std::unordered_map<uint32_t, T> &map, uint32_t handle) {
            auto it = map.find(handle);
            if(it == map.end()) {
                return nullptr;
            }
            return &(*it).second;
        }

        //ShaderGL*   GetShader(ShaderHandle handle);
        uint32_t    GenerateHandle();
        bool        BindAttributes(const VertLayout &vert_layout, const AttribLayout &attrib_layout);
    };
}