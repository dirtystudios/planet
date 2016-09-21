#pragma once

#include <string>
#include "Mesh.h"
#include "RenderObj.h"
#include "StateGroup.h"

class TextRenderer;

class TextRenderObj : public RenderObj {
private:
    friend TextRenderer;

private:
    std::string        _text;
    Mesh               _mesh;
    Mesh               _cursorMesh;
    uint32_t           _cursorPos{0};
    bool               _cursorEnabled{false};
    ConstantBuffer*    _constantBuffer{nullptr};
    float              _posX;
    float              _posY;
    glm::vec3          _textColor;
    std::vector<float> _glyphXOffsets;

    const gfx::StateGroup* _group{nullptr};
    const gfx::StateGroup* _cursorGroup{nullptr};

public:
    TextRenderObj(const std::string& text, float pixelX, float pixelY, const glm::vec3& color)
        : RenderObj(RendererType::Text), _text(text), _posX(pixelX), _posY(pixelY), _textColor(color) {}

    const std::string& text() const { return _text; }

    void text(const std::string& text) { _text = text; }
    void cursorPos(uint32_t pos) { _cursorPos = pos; }
    void cursorEnabled(bool enabled) { _cursorEnabled = enabled; }
};
