#pragma once

#include <vector>
#include <memory>

namespace gfx {
    class SemanticNameCache {
    private:
        static std::vector<std::unique_ptr<const char[]>> m_semanticNameCache;
    public:
        static const char* AddGetSemanticNameToCache(const char* semName);
    };
}