#include "SemanticNameCache.h"

namespace gfx {
    std::vector<std::unique_ptr<const char[]>> SemanticNameCache::m_semanticNameCache = {};

    const char* SemanticNameCache::AddGetSemanticNameToCache(const char* semName) {
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
}