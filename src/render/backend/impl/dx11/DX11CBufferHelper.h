#pragma once
#include <unordered_map>
#include <cstring>

namespace gfx {
    struct CBufferVariable
    {
        size_t  offset;
        size_t  size;
        CBufferVariable()
            : offset(0), size(0) {}
        CBufferVariable(size_t m_offset, size_t m_size)
            : offset(m_offset), size(m_size) {}
    };

    class CBufferDescriptor
    {
    public:
        size_t totalSize = 0;
        // string holds name of variable
        std::unordered_map<std::string, CBufferVariable> details;
        size_t numVars = 0;
        void *bufferData = 0;

        CBufferDescriptor() {};

        void ResetDescriptor(size_t p_size, size_t p_numVars) {
            totalSize = p_size;
            numVars = p_numVars;
            bufferData = calloc(totalSize / 16, 16);
        }

        ~CBufferDescriptor() {
            if (bufferData)
                free(bufferData);
        }

        void AddBufferVariable(const std::string& name, const CBufferVariable& var) {
            details.emplace(std::make_pair(name, var));
        }

        //Note, this isn't bounds/size checked, do it elsewhere
        void UpdateBufferData(const CBufferVariable &var, void* value) {
            void *data = (char *)bufferData + var.offset;
            memcpy(data, value, var.size);
        }

        void UpdateBufferData(const std::string& name, void* value) {
            CBufferVariable var = details[name];
            UpdateBufferData(var, value);
        }

        // Not dealing with this copy pointer bs
        CBufferDescriptor & operator=(const CBufferDescriptor&) = delete;
        CBufferDescriptor(const CBufferDescriptor &other) = delete;



        // quick and dirty stream loading

        friend std::ostream& operator << (std::ostream &os, const CBufferDescriptor &cb) {
            os << cb.totalSize << " ";
            os << cb.numVars << "\n";
            for (auto detail : cb.details) {
                os << detail.first << " " << detail.second.offset << " " << detail.second.size << "\n";
            }
            return os;
        }

        friend std::istream& operator >> (std::istream &is, CBufferDescriptor &cb) {
            is >> cb.totalSize >> cb.numVars;
            for (int x = 0; x < cb.numVars; ++x) {
                CBufferVariable cbVar;
                std::string cbName;
                is >> cbName >> cbVar.offset >> cbVar.size;
                cb.details.emplace(std::make_pair(cbName, cbVar));
            }
            cb.ResetDescriptor(cb.totalSize, cb.numVars);
            return is;
        }
    };
}
