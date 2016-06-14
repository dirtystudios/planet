#pragma once

#include "Renderer.h"
#include <glm/glm.hpp>
#include <map>

struct TextRenderObj : public RenderObj {
    TextRenderObj() : RenderObj(RendererType::Text) {}
    std::string text;
    Mesh mesh;
    ConstantBuffer* constantBuffer;
    glm::vec3 textColor;
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

private:
    // parameters for converting pixel to NDC coordinates
    float _scaleX;
    float _scaleY;

    // parameters for packing glyphs in atlas
    uint32_t _xOffset{0};
    uint32_t _yOffset{0};
    uint32_t _currentRowHeight{0};

    // glyph information
    std::unordered_map<char, Glyph> _loadedGlyphs;

    // gfx resources
    graphics::TextureId _glyphAtlas{0};
    graphics::PipelineStateId _textPS{0};
    graphics::BufferId _vertexBuffer{0};
    size_t _vertexBufferOffset{0};
    size_t _vertexBufferSize{0};
    ConstantBuffer* _viewData{nullptr};

    std::vector<TextRenderObj> _objs;

public:
    TextRenderer(float scaleX = 1.f, float scaleY = 1.f);
    ~TextRenderer();

    void OnInit() final;
    RenderObj* RegisterText(const std::string& text, float pixelX, float pixelY, const glm::vec3& color);
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* queue, RenderView* view) final;
};
