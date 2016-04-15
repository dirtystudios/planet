#pragma once
#include <d3d11.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>
#include <string>

namespace graphics {
    namespace dx11 {
        typedef uint32_t InputLayoutCacheHandle;

        // todo: if this shit enumreader shit works, move it
        template<typename Enum>
        class EnumReader
        {
            Enum& e_;

            friend std::istream& operator >> (std::istream& in, const EnumReader& val) {
                typename std::underlying_type<Enum>::type asInt;
                if (in >> asInt) val.e_ = static_cast<Enum>(asInt);
                return in;
            }
        public:
            EnumReader(Enum& e) : e_(e) {}
        };

        template<typename Enum>
        EnumReader<Enum> read_enum(Enum& e)
        {
            return EnumReader<Enum>(e);
        }

        class DX11SemanticNameCache {
        private:
            // This is to cache semanticstrings, apparently sometimes we lose the reference on windows 7
            static std::vector<std::unique_ptr<const char[]>> m_semanticNameCache;
        public:
            static const char* AddGetSemanticNameToCache(const char* semName) {
                const char* semanticName = nullptr;
                for (auto &name : m_semanticNameCache) {
                    if (!strcmp(name.get(), semName)) {
                        semanticName = name.get();
                        break;
                    }
                }

                // didnt find it
                if (!semanticName) {
                    m_semanticNameCache.emplace_back(_strdup(semName));
                    semanticName = m_semanticNameCache.at(m_semanticNameCache.size() - 1).get();
                }
                return semanticName;
            }
        };

        class DX11InputLayoutDescriptor {
        public:
            std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
            // quick and dirty stream loading

            friend std::ostream& operator << (std::ostream& os, const DX11InputLayoutDescriptor& ild) {
                os << ild.elements.size() << "\n";
                for (auto element : ild.elements) {
                    os << element.AlignedByteOffset << " ";
                    os << element.Format << " ";
                    os << element.InputSlot << " ";
                    os << element.InputSlotClass << " ";
                    os << element.InstanceDataStepRate << " ";
                    os << element.SemanticIndex << " ";
                    os << element.SemanticName << "\n";
                }
                return os;
            }

            friend std::istream& operator >> (std::istream& is, DX11InputLayoutDescriptor &ild) {
                int numVars;
                is >> numVars;
                for (int x = 0; x < numVars; ++x) {
                    D3D11_INPUT_ELEMENT_DESC desc;
                    is >> desc.AlignedByteOffset >> read_enum(desc.Format);
                    is >> desc.InputSlot >> read_enum(desc.InputSlotClass);
                    is >> desc.InstanceDataStepRate >> desc.SemanticIndex;

                    std::string semName;
                    is >> semName;
                    desc.SemanticName =
                        DX11SemanticNameCache::AddGetSemanticNameToCache(semName.c_str());
                    ild.elements.emplace_back(desc);
                }
                return is;
            }
        };

        class DX11InputLayoutCache {
        private:
            struct DX11InputLayoutCacheEntry {
                DX11InputLayoutDescriptor layoutDesc;
                uint32_t numRefs;
            };

            std::unordered_map<InputLayoutCacheHandle, DX11InputLayoutCacheEntry> m_inputLayouts;

        public:
            InputLayoutCacheHandle InsertInputLayout(ID3DBlob* shaderBlob);
            InputLayoutCacheHandle InsertInputLayout(const DX11InputLayoutDescriptor& inputLayout);

            const D3D11_INPUT_ELEMENT_DESC* GetInputLayoutData(InputLayoutCacheHandle handle);
            size_t GetInputLayoutSize(InputLayoutCacheHandle handle);
            void RemoveInputLayout(InputLayoutCacheHandle handle);
            static DX11InputLayoutDescriptor GenerateInputLayout(ID3DBlob* pShaderBlob);
        };
    }
}