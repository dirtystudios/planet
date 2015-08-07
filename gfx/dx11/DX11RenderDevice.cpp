#include "DX11RenderDevice.h"
#include "../../Log.h"
#include <d3dcompiler.h>

#define SafeGet(id, idx) id[idx];

#define DEBUG_DX11

// Debug Helper Functions

// Helper sets a D3D resource name string (used by PIX and debug layer leak reporting).
inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char *name)
{
#ifdef DEBUG_DX11
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(name)), name);
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
#endif
}

// Debug Initializtion

void InitDX11DebugLayer(ID3D11Device* dev){

#ifdef DEBUG_DX11
    ID3D11Debug *d3dDebug = nullptr;
    if (SUCCEEDED(dev->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
    {
        ID3D11InfoQueue *d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // Add more message IDs here as needed
            };

            /*D3D11_INFO_QUEUE_FILTER filter;
            memset(&filter, 0, sizeof(filter));
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);*/
            d3dInfoQueue->Release();
        }
        else {
            LOG_E("DX11Render: Failed to Create d3dDebugInfoQueue.");
            d3dDebug->Release();
        }
    }
    else {
        LOG_E("DX11Render: Failed to Create d3dDebug Interface.");
    }
#endif
}

namespace graphics {
    IndexBufferHandle RenderDeviceDX11::CreateIndexBuffer(void* data, size_t size, BufferUsage usage) {
        ComPtr<ID3D11Buffer> indexBuffer = NULL;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = size;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = { 0 };
        initData.pSysMem = data;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        HRESULT hr = m_dev->CreateBuffer(&bufferDesc, &initData, &indexBuffer);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed creating Index Buffer. HR: 0x%x", hr);
            return 0;
        }
        else {
            uint32_t handle = GenerateHandle();
            IndexBufferDX11 ib;
            ib.indexBuffer = indexBuffer.Get();

            // This set buffer should be moved eventually to its own command
            m_devcon->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            m_indexBuffers.insert(std::make_pair(handle, ib));
            return handle;
        }
    }
    
    void RenderDeviceDX11::DestroyIndexBuffer(IndexBufferHandle handle) {
        auto it = m_indexBuffers.find(handle);
        if(it == m_indexBuffers.end()) {
            LOG_E("DX11RenderDev: Failed to DestroyIndexBuffer: %d", handle)
            return;
        }
        IndexBufferDX11 &ib = (*it).second;
        
        if(ib.indexBuffer)
            ib.indexBuffer->Release();
        m_indexBuffers.erase(it);
    }
    
    VertexBufferHandle RenderDeviceDX11::CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) {
        ComPtr<ID3D11Buffer> vertexBuffer = NULL;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = SafeGet(bufferUsageDX11, (uint32_t)usage);
        bufferDesc.ByteWidth = size * 4;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = data;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        HRESULT hr = m_dev->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed to create Vertex buffer. HResult: 0x%x", hr);
            return 0;
        }
        else {
            uint32_t handle = GenerateHandle();
            VertexBufferDX11 vb = {};
            vb.vertexBuffer = vertexBuffer.Get();
            vb.layout = layout;

            // This set should probly go in its own command
            m_devcon->IASetVertexBuffers(0, 1, &vertexBuffer, &vb.layout.stride, 0);

            m_vertexBuffers.insert(std::make_pair(handle, vb));
            return handle;
        }
    }


    void RenderDeviceDX11::DestroyVertexBuffer(VertexBufferHandle handle) {
        auto it = m_vertexBuffers.find(handle);
        if (it == m_vertexBuffers.end()) {
            LOG_E("DX11RenderDev: Failed to DestroyVertexBuffer: %d", handle)
                return;
        }
        VertexBufferDX11 &vb = (*it).second;

        if (vb.vertexBuffer)
            vb.vertexBuffer->Release();
        m_vertexBuffers.erase(it);
    }
    
    ShaderHandle RenderDeviceDX11::CreateShader(ShaderType shaderType, const char** source) {
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr;

        char *entryPoint;
        char *target;

        switch (shaderType){
        case ShaderType::FRAGMENT_SHADER:
            entryPoint = "PSMain";
            target = "ps_5_0";
            break;
        case ShaderType::VERTEX_SHADER:
            entryPoint = "VSMain";
            target = "vs_5_0";
            break;
        default:
            LOG_E("DX11RenderDev: Unsupported shader type supplied. Type: %d", shaderType);
            return 0;
        }

        hr = D3DCompile(source[0], strlen(source[0]), NULL, NULL, NULL, entryPoint, target, 0, 0, &blob, &errorBlob);

        if (FAILED(hr)){
            if (errorBlob){
                LOG_E("DX11RenderDev: Failed to Compile Shader. Error: %s", errorBlob->GetBufferPointer());
            }
            else {
                LOG_E("DX11RenderDev: Failed to compile Shader. Hr: 0x%x", hr);
            }
            return 0;
        }

        uint32_t handle = GenerateHandle();
        ShaderDX11 shader = {};
        shader.shader = blob.Get();
        shader.type = shaderType;

        m_shaders.insert(std::make_pair(handle, shader));
        return handle;
    }
    
    void RenderDeviceDX11::DestroyShader(ShaderHandle handle) {
        auto it = m_shaders.find(handle);
        if(it == m_shaders.end()) {
            LOG_E("DX11RenderDev: Invalid Shader handle given to destroy.");
            return;
        }
        ShaderDX11 &shader = (*it).second;
        
        if(shader.shader) {
            shader.shader->Release();
        }
        m_shaders.erase(it);
    }
    
    ConstantBufferHandle RenderDeviceDX11::CreateConstantBuffer(const MemoryLayout& layout, void* data, BufferUsage usage) {
        ComPtr<ID3D11Buffer> constantBuffer = NULL;

        // TODO-Jake: use usage and data parameters

        // so VS_CONSTANT_BUFFER is defined global in the renderer parent

        VS_CONSTANT_BUFFER vsConstData;
        //not sure what to do for these yet bleh
        vsConstData.g_view = {};
        vsConstData.g_projection = {};
        vsConstData.g_world = {};
        vsConstData.g_elevations_tile_index = 0;
        vsConstData.g_normals_tile_index = 0;

        D3D11_BUFFER_DESC cbDesc;
        cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = &vsConstData;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;

        HRESULT hr = m_dev->CreateBuffer(&cbDesc, &InitData, &constantBuffer);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Failed to Create ConstantBuffer. Hr: 0x%x", hr);
            return hr;
        }
        uint32_t handle = GenerateHandle();

        ConstantBufferDX11 cb = {};
        cb.constantBuffer = constantBuffer.Get();
        cb.layout = layout;

        m_constantBuffers.insert(std::make_pair(handle, cb));

        return handle;
    }
    
    void RenderDeviceDX11::DestroyConstantBuffer(ConstantBufferHandle handle) {
        auto it = m_constantBuffers.find(handle);
        if(it == m_constantBuffers.end()) {
            LOG_E("DX11RenderDev: Invalid handle given to DestroyConstantBuffer.");
            return;
        }
        ConstantBufferDX11 &cb = (*it).second;
        
        if(cb.constantBuffer) {
            cb.constantBuffer->Release();
        } 
        m_constantBuffers.erase(it);
    }
    
    ProgramHandle RenderDeviceDX11::CreateProgram(ShaderHandle* shaderHandles, uint32_t numShaders) {
        //TODO-Jake: redo this instead of hardcoding the input_layout...oops
        //      ---- Possibly figure out better way to do 'programs'?
        
        HRESULT hr;
        ShaderHandle handle;
        ShaderDX11* shader;
        ProgramDX11 program = { 0 };

        for (uint32_t x = 0; x < numShaders; ++x){
            handle = shaderHandles[x];
            shader = Get(m_shaders, handle);
            if (!shader) {
                LOG_E("DX11RenderDev: Invalid Shader handle given to CreateProgram");
                return 0;
            }

            switch (shader->type) {
            case ShaderType::VERTEX_SHADER:
                if (program.vertexShader){
                    LOG_E("DX11RenderDev: Multiple VertexShaders given in CreateProgram.");
                    return 0;
                }

                hr = m_dev->CreateVertexShader(shader->shader->GetBufferPointer(), shader->shader->GetBufferSize(), NULL, &program.vertexShader);
                if (FAILED(hr)) {
                    LOG_E("DX11RenderDev: Failed to Create VertexShader in CreateProgram. Hr: 0x%x", hr);
                    return 0;
                }

                //this is going here for now

                D3D11_INPUT_ELEMENT_DESC ied[] =
                {
                    { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    //{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };

                hr = m_dev->CreateInputLayout(ied, 2, shader->shader->GetBufferPointer(), shader->shader->GetBufferSize(), &program.inputLayout);
                if (FAILED(hr)) {
                    LOG_E("DX11RenderDev: Failed to Create InputLayout in CreateProgram. Hr: 0x%x", hr);
                    return 0;
                }


                break;
            case ShaderType::FRAGMENT_SHADER:
                if (program.pixelShader){
                    LOG_E("DX11RenderDev: Multiple FragmentShaders given in CreateProgram.");
                    return 0;
                }
                hr = m_dev->CreatePixelShader(shader->shader->GetBufferPointer(), shader->shader->GetBufferSize(), NULL, &program.pixelShader);
                if (FAILED(hr)){
                    LOG_E("DX11RenderDev: Failed to Create PixelShader in CreateProgram. Hr: 0x%x", hr);
                    return 0;
                }
                break;
            default:
                LOG_E("DX11RenderDev: Invalid shader type given to CreateProgram.");
                return 0;
            }
        }

        uint32_t handle = GenerateHandle();
        m_programs.insert(std::make_pair(handle, program));
        return handle;
    }
    
    void RenderDeviceDX11::DestroyProgram(ProgramHandle handle) {
        auto it = m_programs.find(handle);
        if(it == m_programs.end()) {
            LOG_E("DX11RenderDev: Invalid ProgramHandle given to DestroyProgram.");
            return;
        }
        ProgramDX11 &program = (*it).second;
        
        if(program.pixelShader) 
            program.pixelShader->Release();

        if (program.vertexShader)
            program.vertexShader->Release();

        if (program.inputLayout)
            program.inputLayout->Release();
        
        m_programs.erase(it);
    }
    
    TextureHandle RenderDeviceDX11::CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data) {
        // TODO-Jake : Implement this...
        LOG_E("DX11RenderDev: Unimplemented Command CreateTexture2D.");
        return 0;

       /* GLenum gl_tex_format = SafeGet(texture_format_mapping, (uint32_t)tex_format);
        GLenum gl_data_type = SafeGet(data_type_mapping, (uint32_t)data_type);
        GLenum gl_data_format = SafeGet(data_format_mapping, (uint32_t)data_format);

        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, gl_tex_format, width, height, 0, gl_data_format, gl_data_type, data));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
        uint32_t handle = GenerateHandle();
        TextureGL texture = {};
        texture.id = id;
        texture.target = GL_TEXTURE_2D;
        _textures.insert(std::make_pair(handle, texture));
        return handle;*/
    }
    
    TextureHandle RenderDeviceDX11::CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data) {
        //TODO-Jake: Implement this....
        LOG_E("DX11RenderDev: Unimplemented Command CreateTextureCube.");
        return 0;

        /*GLenum gl_tex_format = SafeGet(texture_format_mapping, (uint32_t)tex_format);
        GLenum gl_data_type = SafeGet(data_type_mapping, (uint32_t)data_type);
        GLenum gl_data_format = SafeGet(data_format_mapping, (uint32_t)data_format);

        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
        for(uint32_t side = 0; side < 6; ++side) {
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, gl_tex_format, width, height, 0, gl_data_type, gl_data_format, data[side]));
        }
        
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
        uint32_t handle = GenerateHandle();
        TextureGL texture = {};
        texture.id = id;
        texture.target = GL_TEXTURE_CUBE_MAP;
        _textures.insert(std::make_pair(handle, texture));
        return handle;*/
    }

    TextureHandle RenderDeviceDX11::CreateTexture2DArray(TextureFormat texFormat, DataType dataType, DataFormat dataFormat, uint32_t width, uint32_t height, uint32_t depth, void* data){
        // TODO-Jake: actually use parameters here

        ComPtr<ID3D11Texture2D> texture;
        DXGI_FORMAT dxFormat = SafeGet(textureFormatDX11, (uint32_t)texFormat);

        D3D11_TEXTURE2D_DESC tdesc = { 0 };

        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = 1;
        tdesc.ArraySize = depth;

        // todo thoughts, add commands to set sampledesc? thats antialiasing right?
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = 0;

        tdesc.Format = dxFormat;

		int formatByteSize = 0;
		switch (dataFormat) {
		case DataFormat::RED:
			formatByteSize = 4;
			break;
		case DataFormat::RGB:
			formatByteSize = 12;
			break;
		default:
			LOG_E("DX11RenderDev: Unsupported dataformat given for CreateTexture2DArray.");
			return 0;
		}

		D3D11_SUBRESOURCE_DATA subData = { 0 };
		subData.pSysMem = data;
		subData.SysMemPitch = formatByteSize * width;
		subData.SysMemSlicePitch = formatByteSize * width * height;

		//datatype currently just float

        HRESULT hr = m_dev->CreateTexture2D(&tdesc, NULL, &texture);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed Creating Texture2DArray Hr: 0x%x", hr);
            return 0;
        }

        // TODO-Jake: Still not sure how resourceviews work, so every texture gets one for now :/
        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        hr = m_dev->CreateShaderResourceView(texture.Get(), NULL, &shaderResourceView);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed Creating shaderResourceView in Texture2DArray Hr: 0x%x", hr);
            texture.Reset();
            return 0;
        }

        uint32_t handle = GenerateHandle();
        TextureDX11 textureDX11 = {};
        textureDX11.texture = texture.Get();
        textureDX11.shaderResourceView = shaderResourceView.Get();
        textureDX11.format = dxFormat;
        m_textures.insert(std::make_pair(handle, textureDX11));

        return handle; 
    }

    void RenderDeviceDX11::DestroyTexture(TextureHandle handle) {
        auto it = m_textures.find(handle);
        if(it == m_textures.end()) {
            LOG_E("DX11RenderDev: Invalid handle given to DestroyTexture.");
            return;
        }
        TextureDX11 &texture = (*it).second;
        
        if(texture.texture) 
            texture.texture->Release();
        if (texture.shaderResourceView)
            texture.shaderResourceView->Release();
        
        m_textures.erase(it);
    }

    void RenderDeviceDX11::Clear(float *RGBA) {
        m_devcon->ClearRenderTargetView(renderTarget.Get(), RGBA);
    }

    void RenderDeviceDX11::SwapBuffers() {
        m_swapchain->Present(1, 0);
    }

    void RenderDeviceDX11::UpdateConstantBuffer(ConstantBufferHandle handle, void* data, size_t size, size_t offset) {
        //TODO-Jake: What to do with size and offset?

        ConstantBufferDX11 *cb = Get(m_constantBuffers, handle);
        if (!cb){
            LOG_E("DX11RenderDev: Invalid handle given to UpdateConstantBuffer.");
            return;
        }

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr;
        hr = m_devcon->Map(cb->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed to map constant buffer: %d", handle);
            return;
        }

        VS_CONSTANT_BUFFER* buffData = (VS_CONSTANT_BUFFER*)mappedResource.pData;
        VS_CONSTANT_BUFFER* dataPtr = static_cast<VS_CONSTANT_BUFFER*>(data);

        buffData->g_view = dataPtr->g_view;
        buffData->g_projection = dataPtr->g_projection;
        buffData->g_world = dataPtr->g_world;
        buffData->g_elevations_tile_index = dataPtr->g_elevations_tile_index;
        buffData->g_normals_tile_index = dataPtr->g_normals_tile_index;

        m_devcon->Unmap(cb->constantBuffer, 0);
    }
    
    void RenderDeviceDX11::UpdateTexture(TextureHandle handle, void* data, size_t size) {
        LOG_E("DX11RenderDev: Unimplemented Function UpdateTexture");
    }

    void RenderDeviceDX11::UpdateTexture2DArray(TextureHandle handle, void* data, size_t size, uint32_t width, uint32_t height) {
        TextureDX11 *texture = Get(m_textures, handle);
        if (!texture){
            LOG_E("DX11RenderDev: Invalid handle given to UpdateTexture2DArray.");
            return;
        }

        // TODO-Jake: this ones gunna be hardcoded till i figure out how to make dx11 rendererer actually...well, render
        D3D11_BOX box = { 0 };
        box.left = 0;
        box.top = 0;
        box.front = 0;
        box.back = 1;
        box.right = width;
        box.bottom = height;

        switch (texture->format) {
        case DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT:
            m_devcon->UpdateSubresource(texture->texture, 0, &box, data, width * 1, width * 1 * height);
            break;
        case DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT:
            m_devcon->UpdateSubresource(texture->texture, 0, &box, data, width * 4, width * 4 * height);
            break;
        default:
            LOG_E("DX11Render: Unsupported TextureFormat for UpdateTexture2DArray.");
        }
    }

    void RenderDeviceDX11::BindProgram(ProgramHandle handle) {
        ProgramDX11 *program = Get(m_programs, handle);
        if (!program){
            LOG_E("DX11RenderDev: Invalid handle given to BindProgram.");
            return;
        }

        if (program->pixelShader)
            m_devcon->PSSetShader(program->pixelShader, 0, 0);
        if (program->vertexShader)
            m_devcon->VSSetShader(program->vertexShader, 0, 0);

        if (program->inputLayout)
            m_devcon->IASetInputLayout(program->inputLayout);
    }

    void RenderDeviceDX11::SetRasterizerState(uint32_t state) {

    }

    void RenderDeviceDX11::SetDepthState(uint32_t state) {

    }

    void RenderDeviceDX11::SetBlendState(uint32_t state) {

    }

    void RenderDeviceDX11::DrawArrays(VertexBufferHandle handle, uint32_t startVertex, uint32_t numVertices) {
        // TODO-Jake, need primitive drawtype command?
        m_devcon->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_devcon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), NULL);
        m_devcon->Draw(numVertices, startVertex);
    }

    void RenderDeviceDX11::BindTexture(TextureHandle handle, uint32_t slot) {

    }

    void RenderDeviceDX11::BindSampler(SamplerHandle handle, uint32_t location) {
        // TODO-Jake: FIXME
        SamplerStateDX11 *ss = Get(m_samplers, handle);
        if (!ss){
            LOG_E("DX11RenderDev: Invalid handle given to BindSampler.");
        }
        m_devcon->VSSetSamplers(0, 1, &ss->samplerState1);
        m_devcon->VSSetSamplers(1, 1, &ss->samplerState2);
    }

    SamplerHandle RenderDeviceDX11::CreateSamplers(){
        D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());

        ID3D11SamplerState *samplerState1, *samplerState2;
        HRESULT hr = m_dev->CreateSamplerState(&samplerDesc, &samplerState1);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed creating samplerState HR: 0x%x", hr);
        }

        m_dev->CreateSamplerState(&samplerDesc, &samplerState2);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed creating samplerState HR: 0x%x", hr);
        }

        uint32_t handle = GenerateHandle();
        SamplerStateDX11 samplerDX11 = {};
        samplerDX11.samplerState1 = samplerState1;
        samplerDX11.samplerState2 = samplerState2;
        m_samplers.insert(std::make_pair(handle, samplerDX11));

        return handle;
    }

    void RenderDeviceDX11::DestroySampler(SamplerHandle handle) {
        // TODO-Jake: FIXME

        auto it = m_samplers.find(handle);
        if (it == m_samplers.end()) {
            LOG_E("DX11RenderDev: Invalid handle given to DestroySampler.");
            return;
        }
        SamplerStateDX11 &sampler= (*it).second;

        //if (sampler.samplerState)
            //sampler.samplerState->Release();

        m_samplers.erase(it);
    }

    void RenderDeviceDX11::SetProgramTexture(TextureHandle handle, const char *paramName, uint32_t slot) {
        // TODO-Jake: need shader reflecter for param name?
        // hardcoding this for now

        TextureDX11 *tex = Get(m_textures, handle);
        if (!tex){
            LOG_E("DX11RenderDev: Invalid handle given to SetProgramTexture.");
        }

        m_devcon->VSSetShaderResources(slot, 1, &tex->shaderResourceView);
    }

    void RenderDeviceDX11::BindConstantBuffer(ConstantBufferHandle handle, uint32_t slot) {
        ConstantBufferDX11 *cb = Get(m_constantBuffers, handle);
        if (!cb){
            LOG_E("DX11RenderDev: Invalid handle given to BindConstantBuffer.");
        }
        m_devcon->VSSetConstantBuffers(slot, 1, &cb->constantBuffer);
    }
   
    int RenderDeviceDX11::InitializeDevice(void* args) {
        m_hwnd = static_cast<HWND>(args);

        D3D_FEATURE_LEVEL FeatureLevelsRequested[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        uint32_t numLevelsRequested = 2;
        uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG_DX11
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &m_dev, nullptr, &m_devcon);

        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating D3D11Device: 0x%x", hr);
            return hr;
        }

        InitDX11DebugLayer(m_dev.Get());

        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&m_factory));
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating DXGIFactory: 0x%x", hr);
            return hr;
        }

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.BufferCount = 2;
        sd.OutputWindow = m_hwnd;
        sd.Windowed = true;

        hr = m_factory->CreateSwapChain(m_dev.Get(), &sd, &m_swapchain);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating Swapchain: 0x%x", hr);
            return hr;
        }

        ComPtr<ID3D11Texture2D> backbuffer;
        m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);

        m_dev->CreateRenderTargetView(backbuffer.Get(), nullptr, &renderTarget);

        //SwapBuffers();

        // TODO-Jake: move this to setRasterizerState
        D3D11_RASTERIZER_DESC rasterDesc;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
        //lets try none for testing currently
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FrontCounterClockwise = true;
        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = false;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        ComPtr<ID3D11RasterizerState> rasterState;

        hr = m_dev->CreateRasterizerState(&rasterDesc, &rasterState);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating RasterState: 0x%x", hr);
            return hr;
        }

        m_devcon->RSSetState(rasterState.Get());

        //viewport?
        D3D11_VIEWPORT vp;
        vp.Width = 800.0f;
        vp.Height = 600.0f;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_devcon->RSSetViewports(1, &vp);

        return hr;
    }

    void RenderDeviceDX11::PrintDisplayAdapterInfo() {
        ComPtr<IDXGIAdapter> adapter;
        m_factory->EnumAdapters(0, &adapter);
        auto adapterDesc = DXGI_ADAPTER_DESC();
        adapter->GetDesc(&adapterDesc);

        char buffer[128];
        wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);
        char *dxLevel = "Unknown";
        switch (m_dev->GetFeatureLevel()){
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1:
            dxLevel = "11.1";
            break;
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0:
            dxLevel = "11.0";
            break;
        }

        LOG_D("DirectX Version: v%s", dxLevel);
        LOG_D("DisplayAdaterDesc: %s", buffer);
        LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
    }

    uint32_t RenderDeviceDX11::GenerateHandle() {
        static uint32_t key = 0;
        return ++key;
    }

    /*ShaderGL* RenderDeviceGL::GetShader(ShaderHandle handle) {
        return Get(_shaders, handle);
    }*/
}
