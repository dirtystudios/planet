#pragma once

#include <glm/glm.hpp>

struct ChunkedTerrainVertex {
    glm::vec3 pos;
    glm::vec2 tex;

    static const graphics::VertexLayoutDesc& GetVertexLayoutDesc() {
        static graphics::VertexLayoutDesc layout{
            {{
                 graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Position,
             },
             {
                 graphics::VertexAttributeType::Float2, graphics::VertexAttributeUsage::Texcoord0,
             }}};

        return layout;
    }
};
