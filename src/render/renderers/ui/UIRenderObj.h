#pragma once

#include <glm/glm.hpp>
#include "ConstantBuffer.h"
#include "DrawItemEncoder.h"
#include "RenderObj.h"
#include "VertexStream.h"

class UIRenderer;

class UIFrameRenderObj : public RenderObj {
private:
    friend UIRenderer;

public:
    UIFrameRenderObj(float x, float y, float width, float height, float isShown) : RenderObj(RendererType::Ui), _x(x), _y(y), _w(width), _h(height), _isShown(isShown) {}
    ~UIFrameRenderObj() {}

    float x() const { return _x; };
    float y() const { return _y; };
    float width() const { return _w; }
    float height() const { return _h; }
    bool  isShown() const { return _isShown; }

    void x(float x) { _x = x; };
    void y(float y) { _y = y; }
    void width(float w) { _w = w; }
    void height(float h) { _h = h; }
    void isShown(bool isShown) { _isShown = isShown; }

private:
    float _x{0.f};
    float _y{0.f};
    float _w{0.f};
    float _h{0.f};
    bool  _isShown{true};

    gfx::DrawCall     _drawCall;
    gfx::VertexStream _stream;
    glm::vec4         _bgColor;
    glm::vec4         _borderColor;
    glm::vec2         _borderSize;

    ConstantBuffer*        _frameData{nullptr};
    const gfx::StateGroup* _group{nullptr};
    const gfx::DrawItem*   _item{nullptr};
};
