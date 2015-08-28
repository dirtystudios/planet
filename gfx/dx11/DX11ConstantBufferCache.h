#pragma once
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <memory>
#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include "../ParamType.h"

// This class handles creation, caching and 'updating' of constantBuffer data
// It doesn't call any dev/devcon functions
namespace graphics {

    typedef uint32_t ConstantBufferCacheHandle;

    class DX11ConstantBufferCache {
    private:
        struct CBufferVariable
        {
            size_t  offset;
            size_t  size;
            CBufferVariable(size_t m_offset, size_t m_size)
                : offset(m_offset), size(m_size) {}
        };

        class CBufferDescriptor
        {
        public:
            size_t totalSize;
            std::string name;
            // string holds name of variable
            std::unordered_map<std::string, CBufferVariable> details;
            size_t numVars;
            void *bufferData;

            CBufferDescriptor(size_t m_size, std::string m_name, size_t m_numVars)
                : totalSize(m_size), name(m_name) 
            {
                // alloc with alignment 
                bufferData = calloc(totalSize / 16, 16);
            }

            //Todo: Hoping this is called from unique_ptr, not sure though
            ~CBufferDescriptor() {
                free(bufferData);
            }

            //Note, this isn't bounds/size checked, do it elsewhere
            void UpdateBufferData(const CBufferVariable *var, void* value) {
                void *data = (char *)bufferData + var->offset;
                memcpy(data, value, var->size);
            }

            // Not dealing with this copy pointer bs
            CBufferDescriptor & operator=(const CBufferDescriptor&) = delete;
            CBufferDescriptor(const CBufferDescriptor &other) = delete;
        };

        std::unordered_map<ConstantBufferCacheHandle, std::unique_ptr<CBufferDescriptor>> m_CBuffers;

    public:
        DX11ConstantBufferCache() {};
        ~DX11ConstantBufferCache() {};
        bool InsertConstantBuffer(ID3DBlob* shaderBlob, ConstantBufferCacheHandle handle);
        void UpdateConstantBuffer(ConstantBufferCacheHandle handle, ParamType paramType, const char *paramName, void* data);
        void* GetConstantBufferData(ConstantBufferCacheHandle handle);
        uint32_t GetConstantBufferSize(ConstantBufferCacheHandle handle);
        void RemoveConstantBuffer(ConstantBufferCacheHandle handle);
    };
}