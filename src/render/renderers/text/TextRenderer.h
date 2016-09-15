#pragma once

#include "Renderer.h"
#include "StateGroup.h"
#include "DrawItem.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

struct TextRenderObj : public RenderObj {
    TextRenderObj() : RenderObj(RendererType::Text) {}
    std::string text;
    Mesh mesh;
    Mesh cursorMesh;
    uint32_t cursorPos{0};
    bool cursorEnabled{false};
    ConstantBuffer* constantBuffer;
    glm::vec3 textColor;
    float posX;
    float posY;

    std::vector<float> glyphXOffsets;

    void SetText(std::string newText) {
        text = newText;
    }
    
    void SetCursorPos(uint32_t pos) {
        cursorPos = pos;
    }

    void SetCursorEnabled(bool enabled) {
        cursorEnabled = enabled;
    }

    const gfx::StateGroup* _group{nullptr};
    const gfx::StateGroup* _cursorGroup{nullptr};
};

class TextRenderer : public Renderer {
private:
    struct Rectangle {
        glm::vec2 bl{0, 0};
        glm::vec2 tr{0, 0};

        float GetWidth() { return tr.x - bl.x; }

        float GetHeight() { return tr.y - bl.y; }
    };

    struct Glyph {
        Rectangle region;
        float xAdvance{0};
        float yAdvance{0};
        float xOffset{0};
        float yOffset{0};
        float width{0};
        float height{0};
    };

    const gfx::StateGroup* _base;
    const gfx::StateGroup* _cursorBase;

private:
    // parameters for converting pixel to NDC coordinates
    float _scaleX;
    float _scaleY;

    // parameters for packing glyphs in atlas
    uint32_t _xOffset{0};
    uint32_t _yOffset{0};
    uint32_t _currentRowHeight{0};

    uint32_t _maxGlyphHeight{0};

    // glyph information
    std::unordered_map<char, Glyph> _loadedGlyphs;

    // gfx resources
    gfx::TextureId _glyphAtlas{0};
    gfx::BufferId _vertexBuffer{0};
    gfx::BufferId _cursorBuffer{0};
    size_t _vertexBufferOffset{0};
    size_t _vertexBufferSize{0};
    ConstantBuffer* _viewData{nullptr};

    std::vector<std::unique_ptr<TextRenderObj>> _objs;

    void SetVertices(TextRenderObj* renderObj);
    const gfx::DrawItem* CreateDrawItem(TextRenderObj* renderObj);
    const gfx::DrawItem* CreateCursorDrawItem(TextRenderObj* renderObj);

public:
    TextRenderer(float scaleX = 1.f, float scaleY = 1.f);
    ~TextRenderer();

    void OnInit() final;
    TextRenderObj* RegisterText(const std::string& text, float pixelX, float pixelY, const glm::vec3& color);
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* queue, RenderView* view) final;
};
