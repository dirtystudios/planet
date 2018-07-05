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
#include <cmath>

struct TextViewConstants {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 eyeDir;
};

struct TextConstants {
    glm::vec3 textColor;
};

struct GlyphVertex {
    glm::vec3 pos;
    glm::vec2 tex;
};

struct CursorPosVertex {
    glm::vec3 pos;
};

#ifdef _WIN32
static const std::string kFontPath = "C:/Windows/Fonts/Arial.ttf";
#else
static const std::string kFontPath = "/Library/Fonts/Arial.ttf";
#endif
static const std::string kDefaultGlyphSet =
//    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
" ASG";
// font parameters
static constexpr float normalizationFactor = 128;
static constexpr uint32_t padding           = 128;
static constexpr double kFontSize           = 512;
static constexpr FT_UInt kDpi               = 96;
static constexpr uint32_t kAtlasWidth       = 4096;
static constexpr uint32_t kAtlasHeight      = 4096;
static constexpr size_t kVerticesPerQuad    = 6;
static constexpr size_t kBufferedQuadsCount = 4096; // Maximum number of characters to buffer.
static constexpr size_t kVertexBufferSize   = kBufferedQuadsCount * kVerticesPerQuad * sizeof(GlyphVertex);
static constexpr size_t quadSizeInBytes = 6 * sizeof(GlyphVertex);

static constexpr size_t kCursorBufferSize = 2 * sizeof(CursorPosVertex);


static float *createResampledData(float *inData, size_t width, size_t height, size_t scaleFactor)
{
    size_t scaledWidth = width / scaleFactor;
    size_t scaledHeight = height / scaleFactor;
    float *outData = (float*)malloc(scaledWidth * scaledHeight * sizeof(float));
    
    for (int y = 0; y < height; y += scaleFactor)
    {
        for (int x = 0; x < width; x += scaleFactor)
        {
            float accum = 0;
            for (int ky = 0; ky < scaleFactor; ++ky)
            {
                for (int kx = 0; kx < scaleFactor; ++kx)
                {
                    accum += inData[(y + ky) * width + (x + kx)];
                }
            }
            accum = accum / (scaleFactor * scaleFactor);
            
            outData[(y / scaleFactor) * scaledWidth + (x / scaleFactor)] = accum;
        }
    }
    
    return outData;
}

static float* createSignedDistanceFieldForGrayscaleImage(const uint8_t *imageData, size_t width, size_t height)
{
    if (imageData == NULL || width == 0 || height == 0)
        return NULL;
    
    typedef struct { unsigned short x, y; } intpoint_t;
    
    float *distanceMap = (float*)malloc(width * height * sizeof(float)); // distance to nearest boundary point map
    glm::u16vec2 *boundaryPointMap = (glm::u16vec2*)malloc(width * height * sizeof(glm::u16vec2)); // nearest boundary point map
    
    // Some helpers for manipulating the above arrays
#define image(_x, _y) (imageData[(_y) * width + (_x)] > 0x7f)
#define distance(_x, _y) distanceMap[(_y) * width + (_x)]
#define nearestpt(_x, _y) boundaryPointMap[(_y) * width + (_x)]
    
    const float maxDist = hypot(width, height);
    const float distUnit = 1;
    const float distDiag = sqrt(2);
    
    // Initialization phase: set all distances to "infinity"; zero out nearest boundary point map
    for (long y = 0; y < height; ++y)
    {
        for (long x = 0; x < width; ++x)
        {
            distance(x, y) = maxDist;
            nearestpt(x, y) = (glm::u16vec2){ 0, 0 };
        }
    }
    
    // Immediate interior/exterior phase: mark all points along the boundary as such
    for (long y = 1; y < height - 1; ++y)
    {
        for (long x = 1; x < width - 1; ++x)
        {
            bool inside = image(x, y);
            if (image(x - 1, y) != inside ||
                image(x + 1, y) != inside ||
                image(x, y - 1) != inside ||
                image(x, y + 1) != inside)
            {
                distance(x, y) = 0;
                nearestpt(x, y) = (glm::u16vec2){ x, y };
            }
        }
    }
    
    // Forward dead-reckoning pass
    for (long y = 1; y < height - 2; ++y)
    {
        for (long x = 1; x < width - 2; ++x)
        {
            if (distanceMap[(y - 1) * width + (x - 1)] + distDiag < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x - 1, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x, y - 1) + distUnit < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x + 1, y - 1) + distDiag < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x + 1, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x - 1, y) + distUnit < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x - 1, y);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
        }
    }
    
    // Backward dead-reckoning pass
    for (long y = height - 2; y >= 1; --y)
    {
        for (long x = width - 2; x >= 1; --x)
        {
            if (distance(x + 1, y) + distUnit < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x + 1, y);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x - 1, y + 1) + distDiag < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x - 1, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x, y + 1) + distUnit < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if (distance(x + 1, y + 1) + distDiag < distance(x, y))
            {
                nearestpt(x, y) = nearestpt(x + 1, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
        }
    }
    
    // Interior distance negation pass; distances outside the figure are considered negative
    for (long y = 0; y < height; ++y)
    {
        for (long x = 0; x < width; ++x)
        {
            if (!image(x, y))
                distance(x, y) = -distance(x, y);
                }
    }
    
    free(boundaryPointMap);
    
    return distanceMap;
    
#undef image
#undef distance
#undef nearestpt
}

TextRenderer::TextRenderer(float scaleX, float scaleY)
: TypedRenderer<TextRenderObj>(RendererType::Text)
, _scaleX(scaleX)
, _scaleY(scaleY)
{}

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

        
        
        // get and update texture region
        uint32_t regionWidth  = g->bitmap.width + (padding * 2);
        uint32_t regionHeight = g->bitmap.rows + (padding * 2);
        if (_xOffset + regionWidth >= kAtlasWidth) {
            if (_yOffset + _currentRowHeight >= kAtlasHeight) {
                LOG_E("%s", "texture can't fit anymore glyphs");
                break;
            }

            _yOffset += _currentRowHeight;
            _xOffset          = 0;
            _currentRowHeight = 0;
        }

        glm::vec2 bl = glm::vec2((float)_xOffset / kAtlasWidth, (float)_yOffset / kAtlasHeight);
        glm::vec2 tr = bl + glm::vec2((float)regionWidth / kAtlasWidth, (float)regionHeight / kAtlasHeight);

        Glyph glyph({bl, tr});
        
        // copy to temporary texture buffer
        for (uint32_t row = 0; row < g->bitmap.rows; ++row) {
            uint8_t* offsetBufferPtr = buffer + (kAtlasWidth * (_yOffset + padding + row)) + (_xOffset + padding);

            // glyphs are upside down, need to flip them
            memcpy(offsetBufferPtr, g->bitmap.buffer + (((g->bitmap.rows - 1) - row) * g->bitmap.pitch), sizeof(uint8_t) * g->bitmap.pitch);
        }
        

        // set glyph parameters
        glyph.xOffset     = static_cast<float>(g->bitmap_left);
        glyph.yOffset     = static_cast<float>(g->bitmap_top);
        glyph.xAdvance    = static_cast<float>(g->advance.x >> 6); // 26.6 fractional pixels (1/64th pixels)
        glyph.yAdvance    = static_cast<float>(g->advance.y >> 6); // 26.6 fractional pixels (1/64th pixels)
        glyph.width       = static_cast<float>(g->bitmap.width);
        glyph.height      = static_cast<float>(g->bitmap.rows);
        

        _maxGlyphHeight   = std::max(_maxGlyphHeight, g->bitmap.rows);
        _currentRowHeight = std::max(_currentRowHeight, regionHeight);
        _xOffset += regionWidth;
        _loadedGlyphs.insert(std::make_pair(c, glyph));
    }

    size_t height = 512;
    size_t width = 512;
    size_t scaleFactor = kAtlasWidth / height;
    float* ff = createSignedDistanceFieldForGrayscaleImage(buffer, kAtlasWidth, kAtlasHeight);
    float* f = createResampledData(ff, kAtlasWidth, kAtlasHeight, scaleFactor);
    
    uint32_t sdfpixelCount = height * width;
    uint8_t* sdfBuffer = new uint8_t[sdfpixelCount];
    memset(sdfBuffer, 0, sizeof(uint8_t) * sdfpixelCount);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float dist = f[y * width + x];
            float clampDist = fmax(-normalizationFactor, fmin(dist, normalizationFactor));
            float scaledDist = clampDist / normalizationFactor;
            uint8_t value = ((scaledDist + 1) / 2) * UINT8_MAX;
            sdfBuffer[y * width + x] = value;
        }
    }
    
    
    
    _glyphAtlas = device()->CreateTexture2D(gfx::PixelFormat::R8Unorm, gfx::TextureUsageFlags::ShaderRead, kAtlasWidth, kAtlasHeight, buffer, "TextGlyphAtlas");
    _glyphAtlas = device()->CreateTexture2D(gfx::PixelFormat::R8Unorm, gfx::TextureUsageFlags::ShaderRead, width, height, sdfBuffer, "SDFTextGlyphAtlas");
    assert(_glyphAtlas || "Failed to create glyph atlas");
    free(f);
    free(ff);
    delete[] buffer;
    delete[] sdfBuffer;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    _vertexBufferSize    = kVertexBufferSize;
    _vertexBufferOffset  = 0;
    gfx::BufferDesc desc = 
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, _vertexBufferSize, "textVB");
    _vertexBuffer        = device()->AllocateBuffer(desc);
    assert(_vertexBuffer);

    gfx::BufferDesc cDesc = 
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kCursorBufferSize, "textCursorVB");
    _cursorBuffer         = device()->AllocateBuffer(desc);
    assert(_cursorBuffer);

    _viewData = services()->constantBufferManager()->GetConstantBuffer(sizeof(TextViewConstants), "text2DView");
    assert(_viewData);
    _viewData3D = services()->constantBufferManager()->GetConstantBuffer(sizeof(TextViewConstants), "text3DView");
    assert(_viewData3D);

    gfx::BlendState bs;
    bs.enable       = true;
    bs.srcRgbFunc   = gfx::BlendFunc::SrcAlpha;
    bs.dstRgbFunc   = gfx::BlendFunc::OneMinusSrcAlpha;
    bs.rgbMode      = gfx::BlendMode::Add;
    bs.srcAlphaFunc = gfx::BlendFunc::SrcAlpha;
    bs.dstAlphaFunc = gfx::BlendFunc::OneMinusSrcAlpha;
    bs.alphaMode    = gfx::BlendMode::Add;

    gfx::DepthState depthState;
    depthState.enable = false;

    gfx::StateGroupEncoder encoder;

    encoder.Begin();
    encoder.BindResource(_viewData->GetBinding(1));
    const gfx::StateGroup* bind2D = encoder.End();

    encoder.Begin();
    encoder.BindResource(_viewData3D->GetBinding(1));
    const gfx::StateGroup* bind3D = encoder.End();

    encoder.Begin();
    encoder.SetVertexLayout(services()->vertexLayoutCache()->GetPos3fTex2f());
    encoder.SetBlendState(bs);
    encoder.SetDepthState(depthState);
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "text"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "text"));
    encoder.BindTexture(0, _glyphAtlas, gfx::ShaderStageFlags::AllStages);
    encoder.SetVertexBuffer(_vertexBuffer);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Triangles);
    const gfx::StateGroup* baseBind = encoder.End();

    encoder.Begin();
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3f());
    encoder.SetBlendState(bs);
    encoder.SetDepthState(depthState);
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "cursor"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "cursor"));
    encoder.SetVertexBuffer(_cursorBuffer);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Lines);
    const gfx::StateGroup* cursorBaseBind = encoder.End();

    _base = gfx::StateGroupEncoder::Merge({ bind2D , baseBind });
    _base3D = gfx::StateGroupEncoder::Merge({ bind3D, baseBind });

    _cursorBase = gfx::StateGroupEncoder::Merge({ bind2D, cursorBaseBind });
    _cursorBase3D = gfx::StateGroupEncoder::Merge({ bind3D, cursorBaseBind });

    delete baseBind;
    delete cursorBaseBind;
    delete bind2D;
    delete bind3D;
    
    
}

TextRenderer::~TextRenderer() {}

void TextRenderer::Unregister(TextRenderObj* renderObj) { assert(false); }

void TextRenderer::Register(TextRenderObj* textRenderObj) {
    textRenderObj->_constantBuffer = services()->constantBufferManager()->GetConstantBuffer(sizeof(TextConstants), "textConstants");
    textRenderObj->_cursorPos      = 0;

    gfx::StateGroupEncoder encoder;
    if (textRenderObj->_usePerspective)
        encoder.Begin(_base3D);
    else 
        encoder.Begin(_base);
    encoder.BindResource(textRenderObj->_constantBuffer->GetBinding(2));
    textRenderObj->_group = encoder.End();

    gfx::StateGroupEncoder cEncoder;
    if (textRenderObj->_usePerspective)
        encoder.Begin(_cursorBase3D);
    else
        cEncoder.Begin(_cursorBase);
    cEncoder.BindResource(textRenderObj->_constantBuffer->GetBinding(2));
    textRenderObj->_cursorGroup = cEncoder.End();

    const std::string& text = textRenderObj->_text;

    assert(_vertexBufferOffset + (text.length() * quadSizeInBytes) < _vertexBufferSize);
    _vertexBufferOffset += (text.length() * quadSizeInBytes);
    _objs.push_back(textRenderObj);
}

void TextRenderer::SetVertices(TextRenderObj* renderObj, size_t offset) {
    size_t bufferOffset = offset * sizeof(GlyphVertex);
    assert(offset + (renderObj->_text.length() * quadSizeInBytes) < _vertexBufferSize);

    uint8_t* mapped = device()->MapMemory(_vertexBuffer, gfx::BufferAccess::WriteNoOverwrite);
    assert(mapped);
    GlyphVertex* vertices = reinterpret_cast<GlyphVertex*>(mapped + bufferOffset);

    float    penX = renderObj->_posX;
    float    penY = renderObj->_posY;
    float    penZ = renderObj->_posZ;
    uint32_t idx  = 0;

    renderObj->_glyphXOffsets.clear();
    renderObj->_glyphXOffsets.reserve(renderObj->_text.length());
    renderObj->_glyphXOffsets.emplace_back(penX + 1);

    for (char c : renderObj->_text) {
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
        const float s       = glyph.region.bl().x;
        const float t       = glyph.region.bl().y;
        const float regionW = glyph.region.width();
        const float regionH = glyph.region.height();

        vertices[idx++] = {{vx, vy, penZ}, {s, t}};                                     // bl
        vertices[idx++] = {{vx + quadW, vy, penZ}, {s + regionW, t}};                   // br
        vertices[idx++] = {{vx + quadW, vy + quadH, penZ}, {s + regionW, t + regionH}}; // tr
        vertices[idx++] = {{vx + quadW, vy + quadH, penZ}, {s + regionW, t + regionH}}; // tr
        vertices[idx++] = {{vx, vy + quadH, penZ}, {s, t + regionH}};                   // tl
        vertices[idx++] = {{vx, vy, penZ}, {s, t}};                                     // bl

        penX += glyph.xAdvance * _scaleX;
        penY += glyph.yAdvance * _scaleY;

        renderObj->_glyphXOffsets.emplace_back(penX);
    }

    device()->UnmapMemory(_vertexBuffer);
}

const gfx::DrawItem* TextRenderer::CreateDrawItem(TextRenderObj* renderObj, const gfx::StateGroup* defaults) {

    // 'loop' around the vertexBuffer, this is currently to fix directx, and as this buffer fills,
    //  im sure it's going to break again
    bool wasLessThan = _vertexBufferOffsetCheck > _vertexBufferOffset;

    if ((_vertexBufferOffset + renderObj->_text.length()) * quadSizeInBytes >= _vertexBufferSize) {
        assert(_vertexBufferOffset != 0);
        _vertexBufferOffset = 0;
    }

    gfx::DrawCall drawCall;
    drawCall.type = gfx::DrawCall::Type::Arrays;
    drawCall.startOffset = _vertexBufferOffset;
    drawCall.primitiveCount = renderObj->_text.length() * 6;

    SetVertices(renderObj, _vertexBufferOffset);

    _vertexBufferOffset += (renderObj->_text.length() * 6);

    if (wasLessThan && _vertexBufferOffset > _vertexBufferOffsetCheck) {
        assert(false);
        LOG_E("TextBuffer overwriting itself");
    }

    return gfx::DrawItemEncoder::Encode(device(), drawCall, { renderObj->_group, defaults });
}

const gfx::DrawItem* TextRenderer::CreateCursorDrawItem(TextRenderObj* renderObj, const gfx::StateGroup* defaults) {

    uint32_t cursorPos = renderObj->_cursorPos;
    if (cursorPos > renderObj->_text.length()) {
        LOG_D("[Text] Cursor Pos greater than text length, assuming end of str");
        cursorPos = renderObj->_text.length();
    }


    float cursorX = renderObj->_glyphXOffsets[cursorPos];
    float cursorY = renderObj->_posY;

    uint8_t* mapped = device()->MapMemory(_cursorBuffer, gfx::BufferAccess::Write);
    assert(mapped);
    CursorPosVertex* vertices = reinterpret_cast<CursorPosVertex*>(mapped);

    vertices[0] = {{cursorX, cursorY, 0.f}};
    vertices[1] = {{cursorX, cursorY + _maxGlyphHeight, 0.f}};

    device()->UnmapMemory(_cursorBuffer);

    gfx::DrawCall drawCall;
    drawCall.type           = gfx::DrawCall::Type::Arrays;
    drawCall.primitiveCount = 2;
    drawCall.startOffset = 0;

    return gfx::DrawItemEncoder::Encode(device(), drawCall, { renderObj->_cursorGroup, defaults });
}

void TextRenderer::Submit(RenderQueue* queue, const FrameView* view) {
    TextViewConstants* viewConstants = _viewData->Map<TextViewConstants>();
    viewConstants->projection = view->ortho; // TODO: this should be set by renderView
    viewConstants->view = glm::mat4();
    viewConstants->eyeDir = view->eyePos;
    _viewData->Unmap();

    TextViewConstants* viewConstants3d = _viewData3D->Map<TextViewConstants>();
    viewConstants3d->view = view->view * glm::scale(glm::mat4(), glm::vec3(0.25, 0.25, 0.25));
    viewConstants3d->projection = view->projection;
    viewConstants3d->eyeDir = view->eyePos;
    _viewData3D->Unmap();

    bool drewCursor = false;
    _vertexBufferOffsetCheck = _vertexBufferOffset;

    for (auto& text : _objs) {
        text->_constantBuffer->Map<TextConstants>()->textColor = text->_textColor;
        text->_constantBuffer->Unmap();

        text->_drawItem.reset(CreateDrawItem(text, queue->defaults));
        queue->AddDrawItem(3, text->_drawItem.get()); // TODO:: sortkeys based on pass....text needs to be rendered after sky
        if (text->_cursorEnabled) {
            if (drewCursor)
                LOG_D("[Text] Multiple Cursors detected and unsupported.");
            text->_cursorDrawItem.reset(CreateCursorDrawItem(text, queue->defaults));
            queue->AddDrawItem(2, text->_cursorDrawItem.get());
            drewCursor = true;
        }
    }
}
