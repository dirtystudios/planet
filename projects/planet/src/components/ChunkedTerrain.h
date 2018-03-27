#pragma once

#include "Component.h"
#include "RenderObj.h"
#include <glm/glm.hpp>

class alignas(16) ChunkedTerrain : public Component {
public:
    float size;
    std::function<double(double, double, double)> heightmapGenerator;
    glm::mat4 translation;
    glm::mat4 rotation;
    RenderObj* renderObj { nullptr };

    void* operator new(size_t i) {
        return _mm_malloc(i, 16);
    }

    void operator delete(void* p) {
        _mm_free(p);
    }
};