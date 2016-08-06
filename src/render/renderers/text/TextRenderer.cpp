#include "TextRenderer.h"
#include <ft2build.h>
#define generic FTGeneric
#include FT_FREETYPE_H
#undef generic
#include "glm/detail/type_vec.hpp"
#include "glm/detail/type_vec2.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Log.h"
#include "StateGroupEncoder.h"
#include "DrawItemEncoder.h"

struct TextViewConstants {
    glm::mat4 projection;
};

struct TextConstants {
    glm::vec3 textColor;
};

struct GlyphVertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

#ifdef _WIN32
static const std::string kFontPath = "C:/Windows/Fonts/Arial.ttf";
#else
static const std::string kFontPath = "/Library/Fonts/Arial.ttf";
#endif
static const std::string kDefaultGlyphSet =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
// font parameters
static constexpr double kFontSize           = 12;
static constexpr FT_UInt kDpi               = 96;
static constexpr uint32_t kAtlasWidth       = 128;
static constexpr uint32_t kAtlasHeight      = 128;
static constexpr size_t kVerticesPerQuad    = 6;
static constexpr size_t kBufferedQuadsCount = 1024; // Maximum number of characters to buffer.
static constexpr size_t kVertexBufferSize   = kBufferedQuadsCount * kVerticesPerQuad * sizeof(GlyphVertex);

TextRenderer::TextRenderer(float scaleX, float scaleY) : _scaleX(scaleX), _scaleY(scaleY) {}

void TextRenderer::OnInit() {
    // load freetype glyphs
    FT_Library library;
    FT_Error res = 0;

    res = FT_Init_FreeType(&library);
    if (res != FT_Err_Ok) {
        LOG_E("FreeType Init Failed %d", res);
    }

    FT_Face face = nullptr;
    res = FT_New_Face(library, kFontPath.c_str(), 0, &face);
    if (res != FT_Err_Ok) {
        LOG_E("Failed to load face: %s %d", kFontPath.c_str(), res);
    }

    FT_F26Dot6 charWidth  = (int)kFontSize << 6; // 26.6 fractional points (1/64th points)
    FT_F26Dot6 charHeight = (int)kFontSize << 6; // 26.6 fractional points (1/64th points)
    res = FT_Set_Char_Size(face, charWidth, charHeight, kDpi, kDpi);
    if (res != FT_Err_Ok) {
        LOG_E("Freetype failure %d", res);
    }

    uint32_t pixelCount = kAtlasHeight * kAtlasWidth;
    uint8_t* buffer = new uint8_t[pixelCount];
    memset(buffer, 0, sizeof(uint8_t) * pixelCount);

    for (char c : kDefaultGlyphSet) {
        res = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (res != FT_Err_Ok) {
            LOG_E("Failed to load glyph: %c %d", (char)c, res);
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        if (!g->bitmap.buffer && c != ' ') {
            LOG_W("No bitmap buffer for glyph '%c'", (char)c);
            continue;
        }

        Glyph glyph = {};

        // get and update texture region
        uint32_t regionWidth  = g->bitmap.width;
        uint32_t regionHeight = g->bitmap.rows;
        if (_xOffset + regionWidth >= kAtlasWidth) {
            if (_yOffset + _currentRowHeight >= kAtlasHeight) {
                LOG_E("%s", "texture can't fit anymore glyphs");
                break;
            }

            _yOffset += _currentRowHeight;
            _xOffset          = 0;
            _currentRowHeight = 0;
        }

        glyph.region.bl = glm::vec2((float)_xOffset / kAtlasWidth, (float)_yOffset / kAtlasHeight);
        glyph.region.tr =
            glyph.region.bl + glm::vec2((float)regionWidth / kAtlasWidth, (float)regionHeight / kAtlasHeight);

        // copy to temporary texture buffer
        for (uint32_t row = 0; row < g->bitmap.rows; ++row) {
            uint8_t* offsetBufferPtr = buffer + (kAtlasWidth * (_yOffset + row)) + (_xOffset);

            // glyphs are upside down, need to flip them
            memcpy(offsetBufferPtr, g->bitmap.buffer + (((g->bitmap.rows - 1) - row) * g->bitmap.pitch),
                   sizeof(uint8_t) * g->bitmap.pitch);
        }

        // set glyph parameters
        glyph.xOffset     = static_cast<float>(g->bitmap_left);
        glyph.yOffset     = static_cast<float>(g->bitmap_top);
        glyph.xAdvance    = static_cast<float>(g->advance.x >> 6); // 26.6 fractional pixels (1/64th pixels)
        glyph.yAdvance    = static_cast<float>(g->advance.y >> 6); // 26.6 fractional pixels (1/64th pixels)
        glyph.width       = static_cast<float>(g->bitmap.width);
        glyph.height      = static_cast<float>(g->bitmap.rows);
        _currentRowHeight = std::max(_currentRowHeight, regionHeight);
        _xOffset += regionWidth;
        _loadedGlyphs.insert(std::make_pair(c, glyph));
    }

    _glyphAtlas = GetRenderDevice()->CreateTexture2D(gfx::TextureFormat::R_U8, kAtlasWidth, kAtlasHeight, buffer);
    assert(_glyphAtlas || "Failed to create glyph atlas");

    delete[] buffer;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    _vertexBufferSize   = kVertexBufferSize;
    _vertexBufferOffset = 0;
    gfx::BufferDesc desc =
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, _vertexBufferSize);
    _vertexBuffer = GetRenderDevice()->AllocateBuffer(desc);
    assert(_vertexBuffer);

    _viewData = GetConstantBufferManager()->GetConstantBuffer(sizeof(TextViewConstants));
    assert(_viewData);

    gfx::BlendState bs;
    bs.enable       = true;
    bs.srcRgbFunc   = gfx::BlendFunc::SrcAlpha;
    bs.dstRgbFunc   = gfx::BlendFunc::OneMinusSrcAlpha;
    bs.srcAlphaFunc = gfx::BlendFunc::One;
    bs.dstAlphaFunc = gfx::BlendFunc::Zero;

    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexLayout(GetVertexLayoutCache()->GetPos2fTex2f());
    encoder.SetBlendState(bs);
    encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "text"));
    encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "text"));
    encoder.BindResource(_viewData->GetBinding(1));
    encoder.BindTexture(0, _glyphAtlas);
    encoder.SetVertexBuffer(_vertexBuffer);
    _base = encoder.End();
}

TextRenderer::~TextRenderer() {}

RenderObj* TextRenderer::Register(SimObj* simObj) {
    assert(false);
    return nullptr;
}
void TextRenderer::Unregister(RenderObj* renderObj) { assert(false); }

RenderObj* TextRenderer::RegisterText(const std::string& text, float pixelX, float pixelY, const glm::vec3& color) {
    _objs.emplace_back();
    TextRenderObj* textRenderObj = &_objs.back();

    size_t bufferOffset    = _vertexBufferOffset;
    size_t quadSizeInBytes = 6 * sizeof(GlyphVertex);
    assert(bufferOffset + (text.length() * quadSizeInBytes) < _vertexBufferSize);

    uint8_t* mapped = GetRenderDevice()->MapMemory(_vertexBuffer, gfx::BufferAccess::Write);
    assert(mapped);
    GlyphVertex* vertices = reinterpret_cast<GlyphVertex*>(mapped + bufferOffset);

    float penX   = pixelX;
    float penY   = pixelY;
    uint32_t idx = 0;
    for (char c : text) {
        if (c == '\n') {
            assert(false || "unsupported");
        }
        std::unordered_map<char, Glyph>::iterator it = _loadedGlyphs.find(c);
        if (it == _loadedGlyphs.end()) {
            LOG_E("Unloaded glyph '%c'", c);
            continue;
        }

        Glyph& glyph = it->second;

        const float vx      = penX + glyph.xOffset * _scaleX;
        const float vy      = penY - (glyph.height - glyph.yOffset) * _scaleY;
        const float quadW   = glyph.width * _scaleX;
        const float quadH   = glyph.height * _scaleY;
        const float s       = glyph.region.bl.x;
        const float t       = glyph.region.bl.y;
        const float regionW = glyph.region.GetWidth();
        const float regionH = glyph.region.GetHeight();

        vertices[idx++] = {{vx, vy}, {s, t}}; // bl
        vertices[idx++] = {{vx + quadW, vy}, {s + regionW, t}}; // br
        vertices[idx++] = {{vx + quadW, vy + quadH}, {s + regionW, t + regionH}}; // tr
        vertices[idx++] = {{vx + quadW, vy + quadH}, {s + regionW, t + regionH}}; // tr
        vertices[idx++] = {{vx, vy + quadH}, {s, t + regionH}}; // tl
        vertices[idx++] = {{vx, vy}, {s, t}}; // bl

        penX += glyph.xAdvance * _scaleX;
        penY += glyph.yAdvance * _scaleY;
    }

    GetRenderDevice()->UnmapMemory(_vertexBuffer);

    textRenderObj->mesh.vertexBuffer = _vertexBuffer;
    textRenderObj->mesh.vertexStride = sizeof(GlyphVertex);
    textRenderObj->mesh.vertexOffset = _vertexBufferOffset / sizeof(GlyphVertex);
    textRenderObj->mesh.vertexCount  = 6 * text.length();
    textRenderObj->constantBuffer    = GetConstantBufferManager()->GetConstantBuffer(sizeof(TextConstants));
    textRenderObj->textColor = color;
    _vertexBufferOffset += (text.length() * quadSizeInBytes);

    gfx::StateGroupEncoder encoder;
    encoder.Begin(_base);
    encoder.BindResource(textRenderObj->constantBuffer->GetBinding(2));
    textRenderObj->_group = encoder.End();

    gfx::DrawCall drawCall;
    drawCall.type           = gfx::DrawCall::Type::Arrays;
    drawCall.primitiveCount = textRenderObj->mesh.vertexCount;
    drawCall.offset         = textRenderObj->mesh.vertexOffset;

    textRenderObj->_item = gfx::DrawItemEncoder::Encode(GetRenderDevice(), drawCall, &textRenderObj->_group, 1);

    return textRenderObj;
}

void TextRenderer::Submit(RenderQueue* queue, RenderView* view) {
    _viewData->Map<TextViewConstants>()->projection =
        glm::ortho(0.0f, view->viewport->width, 0.0f, view->viewport->height); // TODO: this should be set by renderView
    _viewData->Unmap();

    for (const TextRenderObj& text : _objs) {
        text.constantBuffer->Map<TextConstants>()->textColor = text.textColor;
        text.constantBuffer->Unmap();

        queue->AddDrawItem(2, text._item); // TODO:: sortkeys based on pass....text needs to be rendered after sky
    }
}
