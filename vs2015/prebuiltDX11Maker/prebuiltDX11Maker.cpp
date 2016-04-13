// quick compiler for making prebuilt things 

#include <cstring>
#include <string>
#include <iostream>

#include <windows.h>
#include "Shlwapi.h"

#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include "xxhash.h"

#include "RenderDevice.h"

#include "DX11InputLayoutCache.h"
#include "DX11ConstantBufferHelpers.h"

// be sure to put * at the end.....
const static std::string SHADER_PATH = "C:/Users/irgryo/Documents/gitrepos/planet/src/rendering/shaders/DX11/*";
const static std::string OUTPUT_PATH = "prebuiltOutput/";

graphics::ShaderType g_shaderType;

#ifdef _DEBUG
#define DEBUG_DX11
#endif

static std::string ReadFileContents(const std::string& fpath) {
    std::ifstream fin(fpath);

    if (fin.fail()) {
        std::cout << "Failed to open file " << fpath << std::endl;
        return "";
    }

    std::string ss((std::istreambuf_iterator<char>(fin)),
        std::istreambuf_iterator<char>());

    return ss;
}

ComPtr<ID3DBlob> CompileShader(std::string source) {
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;
    char *entryPoint;
    char *target;

    HRESULT hr;

    // hack to tell which shader is which
    std::size_t findMain;

    findMain = source.find("PSMain");
    if (findMain != std::string::npos) {
        g_shaderType = graphics::ShaderType::FRAGMENT_SHADER;
    }
    else {
        g_shaderType = graphics::ShaderType::VERTEX_SHADER;
    }
    // ---- Compile Shader Source 
    switch (g_shaderType) {
    case graphics::ShaderType::FRAGMENT_SHADER:
        entryPoint = "PSMain";
        target = "ps_5_0";
        break;
    case graphics::ShaderType::VERTEX_SHADER:
        entryPoint = "VSMain";
        target = "vs_5_0";
        break;
    default:
        std::cout << "DX11RenderDev: Unsupported shader type supplied. Type:" <<  (int)g_shaderType << std::endl;
        return 0;
    }

    uint32_t flags = 0;
#ifdef DEBUG_DX11
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;
#endif 

    hr = D3DCompile(source.c_str(), source.length(), NULL, NULL, NULL, entryPoint, target, flags, 0, &blob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
             std::cout << "DX11RenderDev: Failed to Compile Shader. Error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        else {
            std::cout << "DX11RenderDev: Failed to compile Shader. Hr: 0x" << std::hex << hr << std::endl;
        }
        return 0;
    }
    return blob;
}

int CreatePrebuilt(ComPtr<ID3DBlob> shaderBlob, std::string shaderName) {
    uint32_t shaderHash = XXH32(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0);

    std::string shaderPath = OUTPUT_PATH + shaderName + ".cso";

    std::ofstream shaderStream(shaderPath, std::ios::out | std::ios::binary);
    if (shaderStream.fail()) {
        std::cout << "Failed to output compiled shader " << shaderPath << "\nCheck that '" << OUTPUT_PATH << "' directory is made. " << std::endl;
        return 1;
    }

    shaderStream.write((char *)shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
    shaderStream.close();

    size_t numCBuffers = 0;
    graphics::dx11::CBufferDescriptor* cBuffers = graphics::dx11::GenerateConstantBuffer(shaderBlob.Get(), &numCBuffers);

    if (numCBuffers != 0) {
        if (numCBuffers > 1 || cBuffers == 0)
            std::cout << "Currently only 1 CBuffer supported. ignoring multiple. \n" << std::endl;

        shaderStream.open(OUTPUT_PATH + std::to_string(shaderHash) + ".cb", std::ios::out | std::ios::binary);
        if (shaderStream.fail()) {
            std::cout << "cbuffer open faile" << std::endl;
            return 1;
        }

        shaderStream << cBuffers[0];
        shaderStream.close();
    }

    if (g_shaderType == graphics::ShaderType::VERTEX_SHADER) {
        graphics::dx11::DX11InputLayoutDescriptor ilDesc = graphics::dx11::DX11InputLayoutCache::GenerateInputLayout(shaderBlob.Get());
        shaderStream.open(OUTPUT_PATH + std::to_string(shaderHash) + ".il", std::ios::out | std::ios::binary);
        if (shaderStream.fail()) {
            std::cout << "inputlayout file open faile" << std::endl;
            return 1;
        }

        shaderStream << ilDesc;
        shaderStream.close();
    }

    return 0;
}

int main()
{
#ifdef DEBUG_DX11
    std::cout << "DEBUG BUILD, each run generates different hashed shaders, keep that in mind...." << std::endl;
#endif

    WIN32_FIND_DATA search_data;
    memset(&search_data, 0, sizeof(WIN32_FIND_DATA));

    HANDLE handle = FindFirstFile(SHADER_PATH.c_str(), &search_data);

    std::cout << "Building stuff... \n";
    while (handle != INVALID_HANDLE_VALUE)
    {
        if ((search_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            std::string shaderFilename(search_data.cFileName);
            std::cout << "\nProcessing Shader: " << shaderFilename << std::endl;
            std::string shaderSource = ReadFileContents(SHADER_PATH.substr(0, SHADER_PATH.length()-1) + shaderFilename);
            if (shaderSource == "") {
                std::cout << "open file error.." << std::endl;
            }
            else {
                ComPtr<ID3DBlob> temp = CompileShader(shaderSource);
                if (temp.Get() != 0) {
                    if (CreatePrebuilt(temp, shaderFilename.substr(0, shaderFilename.length() -5))) {
                        std::cout << "CreatePrebuilt Shader Error" << std::endl;
                    }
                }
                else {
                    std::cout << "Shader Compile Error!" << std::endl;
                }
            }
            std::cout << "Shader Done!" << std::endl;
        }
        if (FindNextFile(handle, &search_data) == FALSE)
            break;
    }

    //Close the handle after use or memory/resource leak
    FindClose(handle);
    std::cout << "Done! Press Enter to Exit";
    std::string wait;
    std::cin >> wait;

    return 0;
}

