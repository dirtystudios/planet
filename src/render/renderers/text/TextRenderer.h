#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include "DrawItem.h"
#include "Rectangle.h"
#include "Renderer.h"
#include "StateGroup.h"
#include "TextRenderObj.h"

class TextRenderer : public TypedRenderer<TextRenderObj> {
private:
    struct Glyph {
        Glyph(const dm::Rect2Df& region) : region(region) {}

        const dm::Rect2Df region;
        float             xAdvance{0};
        float             yAdvance{0};
        float             xOffset{0};
        float             yOffset{0};
        float             width{0};
        float             height{0};
    };

    const gfx::StateGroup* _base;
    const gfx::StateGroup* _base3D;
    const gfx::StateGroup* _cursorBase;
    const gfx::StateGroup* _cursorBase3D;

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
    gfx::TextureId  _glyphAtlas{0};
    gfx::BufferId   _vertexBuffer{0};
    gfx::BufferId   _cursorBuffer{0};
    size_t          _vertexBufferOffset{0};
    size_t          _vertexBufferOffsetCheck{0};
    size_t          _vertexBufferSize{0};
    ConstantBuffer* _viewData{nullptr};
    ConstantBuffer* _viewData3D{ nullptr };

    std::vector<TextRenderObj*> _objs;

    void SetVertices(TextRenderObj* renderObj, size_t offset);
    const gfx::DrawItem* CreateDrawItem(TextRenderObj* renderObj);
    const gfx::DrawItem* CreateCursorDrawItem(TextRenderObj* renderObj);

public:
    TextRenderer(float scaleX = 1.f, float scaleY = 1.f);
    ~TextRenderer();

    void OnInit() final;

    void Register(TextRenderObj* textRO);
    void Unregister(TextRenderObj* textRO);

    void Submit(RenderQueue* queue, const FrameView* view) final;
};
