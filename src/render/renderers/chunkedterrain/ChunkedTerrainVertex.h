#pragma once

#include <glm/glm.hpp>

struct ChunkedTerrainVertex {
    glm::vec3 pos;
    glm::vec2 tex;

    static const gfx::VertexLayoutDesc& GetVertexLayoutDesc() {
        static gfx::VertexLayoutDesc layout{
            {{
                 gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position,
             },
             {
                 gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0,
             }}};

        return layout;
    }
};
