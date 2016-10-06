#pragma once

#include <glm/glm.hpp>
#include "ConstantBuffer.h"
#include "DrawItemEncoder.h"
#include "RenderObj.h"
#include "VertexStream.h"

class UIRenderer;

class alignas(16) UIFrameRenderObj : public RenderObj {
private:
    friend UIRenderer;

public:
    UIFrameRenderObj(float x, float y, float z, float width, float height, glm::vec3 rot, bool isRendered) : RenderObj(RendererType::Ui), _x(x), _y(y), _z(z), _w(width), _h(height), _isRendered(isRendered) {}
    ~UIFrameRenderObj() {}

    float x() const { return _x; };
    float y() const { return _y; };
    float z() const { return _z; };
    glm::vec3 rot() const { return _rot; };
    float width() const { return _w; }
    float height() const { return _h; }
    bool  isRendered() const { return _isRendered; }

    void x(float x) { _x = x; }
    void y(float y) { _y = y; }
    void z(float z) { _z = z; }
    void rot(const glm::vec3& rot) { _rot = rot; };
    void width(float w) { _w = w; }
    void height(float h) { _h = h; }
    void isRendered(bool isRendered) { _isRendered = isRendered; }

    void* operator new(size_t i) {
        return _mm_malloc(i, 16);
    }

    void operator delete(void* p) {
        _mm_free(p);
    }

private:
    float _x{0.f};
    float _y{0.f};
    float _z{0.f};
    float _w{0.f};
    float _h{0.f};
    glm::vec3 _rot{ 0.f, 0.f, 0.f };
    bool  _isRendered{true};

    gfx::DrawCall     _drawCall;
    gfx::VertexStream _stream;
    glm::vec4         _bgColor;
    glm::vec4         _borderColor;
    glm::vec2         _borderSize;

    ConstantBuffer*        _frameData{nullptr};
    const gfx::StateGroup* _group{nullptr};
    const gfx::DrawItem*   _item{nullptr};
};
