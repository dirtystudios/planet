#pragma once

#include "MeshPart.h"
#include <vector>

class Mesh {
private:
    std::vector<MeshPart> m_parts;
public:
    Mesh(std::vector<MeshPart>&& parts) {
        m_parts = std::move(parts);
    }
    const std::vector<MeshPart>& GetParts() const {
        return m_parts;
    }

};

using MeshPtr = std::shared_ptr<Mesh>;
