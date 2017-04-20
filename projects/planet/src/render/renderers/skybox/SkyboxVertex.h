#pragma once

#include <glm/glm.hpp>

struct SkyboxVertex {
    glm::vec3 pos;

    static const gfx::VertexLayoutDesc& GetVertexLayoutDesc() {
        static gfx::VertexLayoutDesc layout{{{gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float}}};

        return layout;
    }
};
