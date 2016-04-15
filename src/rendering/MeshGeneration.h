#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace meshGen {

using VertexDelegate = std::function<void(float, float, float, float, float)>;

static void GenerateGrid(float centerX, float centerY, float centerZ, float sizeX, float sizeY, uint32_t resolutionX,
                  uint32_t resolutionY, VertexDelegate delegate) {
    float half_sizeX = sizeX / 2.f;
    float half_sizeY = sizeY / 2.f;

    float dx = sizeX / static_cast<float>(resolutionX - 1);
    float dy = sizeY / static_cast<float>(resolutionY - 1);

    auto generateVertex = [&](uint32_t i, uint32_t j) {
        float x         = centerX - half_sizeX + (j * dx);
        float y         = centerY + half_sizeY - (i * dy);
        float z         = centerZ;
        float u         = j * dx / sizeX;
        float v = i * dy / sizeY;
        delegate(x, y, z, u, v);
    };

    for (uint32_t i = 0; i < resolutionY - 1; ++i) {
        for (uint32_t j = 0; j < resolutionX - 1; ++j) {
            generateVertex(i, j);
            generateVertex(i + 1, j);
            generateVertex(i + 1, j + 1);
            generateVertex(i + 1, j + 1);
            generateVertex(i, j + 1);
            generateVertex(i, j);
        }
    }
}

static void GenerateGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,
                  VertexDelegate delegate) {
    GenerateGrid(center.x, center.y, center.z, size.x, size.y, resolution.x, resolution.y, delegate);
}

static void GenerateCube(VertexDelegate delegate, float originX, float originY, float originZ, float scale = 1.f, bool flip = true) {
    static float positions[6][6][3] = {
        {{-0.5, -0.5, 0.5},
         {0.5, -0.5, 0.5},
         {0.5, 0.5, 0.5},
         {0.5, 0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, -0.5, 0.5}}, // front

        {{0.5, -0.5, -0.5},
         {-0.5, -0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, -0.5, -0.5}}, // back

        {{-0.5, -0.5, -0.5},
         {-0.5, -0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, -0.5, -0.5}}, // left

        {{0.5, -0.5, 0.5},
         {0.5, -0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, 0.5},
         {0.5, -0.5, 0.5}}, // right

        {{-0.5, 0.5, 0.5},
         {0.5, 0.5, 0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, 0.5, 0.5}}, // top

        {{0.5, -0.5, 0.5},
         {-0.5, -0.5, 0.5},
         {-0.5, -0.5, -0.5},
         {-0.5, -0.5, -0.5},
         {0.5, -0.5, -0.5},
         {0.5, -0.5, 0.5}} // bottom
    };

    static float texCoords[6][2] = {
        {0.f, 0.f }, //bl
        {1.f, 0.f }, //br
        {1.f, 1.f }, //tr
        {1.f, 1.f }, //tr
        {0.f, 1.f }, //tl
        {0.f, 0.f }  //bl
    };

    for (uint32_t f = 0; f < 6; ++f) { // for each face
        for (uint32_t v = 0; v < 6; ++v) { // for each vertex
            delegate(scale * positions[f][flip ? 5 - v : v][0] + originX, 
                     scale * positions[f][flip ? 5 - v : v][1] + originY,
                     scale * positions[f][flip ? 5 - v : v][2] + originZ,
                     texCoords[v][0],
                     texCoords[v][1]);            
        }
    }
}
}