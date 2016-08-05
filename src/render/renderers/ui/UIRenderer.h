#pragma once
#include "RenderDevice.h"
#include "Config.h"
#include "Helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "UIFrame.h"
#include "StateGroupEncoder.h"
#include "DrawItemEncoder.h"
#include "VertexStream.h"

struct UIViewConstants {
    glm::mat4 projection;
};

struct UIFrameConstants {
    glm::vec4 bgColor;
    glm::vec4 borderColor;
    glm::vec2 borderSize;
};

struct FrameVertex {
    glm::vec4 position;
};

struct alignas(16) UIFrameRenderObj : public RenderObj {
    UIFrameRenderObj() : RenderObj(RendererType::Ui) {}
    gfx::DrawCall drawCall;
    gfx::VertexStream stream;
    glm::vec4 bgColor;
    glm::vec4 borderColor;
    glm::vec2 borderSize;

    ConstantBuffer* frameData{nullptr};
	ui::UIFrame* frame;
    const gfx::StateGroup* group{nullptr};
    const gfx::DrawItem* item{nullptr};

    void* operator new(size_t i) { return _mm_malloc(i, 16); }

    void operator delete(void* p) { _mm_free(p); }
};

class UIRenderer : public Renderer {
private:
    static constexpr size_t kDefaultVertexBufferSize = sizeof(FrameVertex) * 18;

    gfx::BufferId _vertexBuffer;
    size_t _bufferOffset;
    size_t _bufferSize;
    ConstantBuffer* _viewData{nullptr};

	Viewport _lastViewPort{};

    std::vector<UIFrameRenderObj*> _objs;
    const gfx::StateGroup* _base{nullptr};

public:
    void OnInit() {
        _viewData = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIViewConstants));

        _vertexBuffer = GetRenderDevice()->AllocateBuffer(gfx::BufferType::VertexBuffer, kDefaultVertexBufferSize,
                                                          gfx::BufferUsage::Static);
        _bufferOffset = 0;
        _bufferSize   = kDefaultVertexBufferSize;

        gfx::BlendState blendState;
        blendState.enable       = true;
        blendState.srcRgbFunc   = gfx::BlendFunc::SrcAlpha;
        blendState.srcAlphaFunc = blendState.srcRgbFunc;
        blendState.dstRgbFunc   = gfx::BlendFunc::OneMinusSrcAlpha;
        blendState.dstAlphaFunc = blendState.dstRgbFunc;

        gfx::RasterState rasterState;
        rasterState.fillMode = gfx::FillMode::Solid;
        rasterState.cullMode = gfx::CullMode::Back;
        rasterState.windingOrder = gfx::WindingOrder::FrontCCW;

        gfx::DepthState depthState;
        depthState.enable = false;

        gfx::StateGroupEncoder encoder;
        encoder.Begin();
        encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "ui"));
        encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "ui"));
        gfx::VertexLayoutDesc vld = {{{
            gfx::VertexAttributeType::Float4, gfx::VertexAttributeUsage::Position,
        }}};
        encoder.SetVertexLayout(GetVertexLayoutCache()->Get(vld));
        encoder.SetBlendState(blendState);
        encoder.SetDepthState(depthState);
        encoder.SetRasterState(rasterState);
        encoder.BindResource(_viewData->GetBinding(1));
        encoder.SetVertexBuffer(_vertexBuffer);
        _base = encoder.End();
        assert(_vertexBuffer);
        assert(_viewData);
    }

    ~UIRenderer() {
        // TODO
    }

    RenderObj* Register(SimObj* simObj) final {
        assert(false);
        return nullptr;
    }

    // hopefully temporary work around until we figure out object model
	UIFrameRenderObj* RegisterFrame(ui::UIFrame* uiframe,  ui::FrameScale& frame) {
        UIFrameRenderObj* uiRenderObj = new UIFrameRenderObj();
		uiRenderObj->frame = uiframe;
        float w    = static_cast<float>(frame.width);
        float h    = static_cast<float>(frame.height);
        float xpos = frame.x;
        float ypos = frame.y;

        FrameVertex vertices[6] = {{{xpos, ypos + h, 0.0, 0.0}}, {{xpos, ypos, 0.0, 1.0}},
                                   {{xpos + w, ypos, 1.0, 1.0}}, {{xpos, ypos + h, 0.0, 0.0}},
                                   {{xpos + w, ypos, 1.0, 1.0}}, {{xpos + w, ypos + h, 1.0, 0.0}}};

        size_t verticesSize = sizeof(vertices);
        assert(_bufferOffset + verticesSize < _bufferSize);
        assert(w != 0 && h != 0);
        glm::vec2 borderWidth = {2.f, 2.f};
        glm::vec2 pixelSize   = {1.f / w, 1.f / h};

        uiRenderObj->stream.vertexBuffer     = _vertexBuffer;
        uiRenderObj->stream.offset           = 0;
        uiRenderObj->stream.stride           = sizeof(FrameVertex);
        uiRenderObj->bgColor                 = {0.f, 0.f, 0.f, 0.5f};
        uiRenderObj->borderColor             = {0.2f, 0.2f, 0.2f, 0.8f};
        uiRenderObj->borderSize              = pixelSize * borderWidth;
        uiRenderObj->drawCall.type           = gfx::DrawCall::Type::Arrays;
        uiRenderObj->drawCall.primitiveCount = 6;
        uiRenderObj->drawCall.offset         = _bufferOffset / sizeof(FrameVertex);

        // pack all frames into a single buffer
        uint8_t* mapped = GetRenderDevice()->MapMemory(_vertexBuffer, gfx::BufferAccess::WriteInit);
        assert(mapped);
        memcpy(&mapped[_bufferOffset], &vertices, verticesSize);
        GetRenderDevice()->UnmapMemory(_vertexBuffer);
        _bufferOffset += verticesSize;

        uiRenderObj->frameData = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIFrameConstants));

        gfx::StateGroupEncoder encoder;
        encoder.Begin(_base);
        encoder.BindResource(uiRenderObj->frameData->GetBinding(2));
        uiRenderObj->group = encoder.End();

        uiRenderObj->item =
            gfx::DrawItemEncoder::Encode(GetRenderDevice(), uiRenderObj->drawCall, &uiRenderObj->group, 1);

        _objs.push_back(uiRenderObj);
        return uiRenderObj;
    }

    void Unregister(RenderObj* renderObj) final {
        auto it =
            std::find_if(begin(_objs), end(_objs), [&renderObj](UIFrameRenderObj* obj) { return obj == renderObj; });
        if (it != _objs.end()) {
            _objs.erase(it);
        }
    }

    void Submit(RenderQueue* renderQueue, RenderView* renderView) final {

        glm::mat4 projection = glm::ortho(0.0f, renderView->viewport->width, 0.0f, renderView->viewport->height);

        _viewData->Map<UIViewConstants>()->projection = projection;
        _viewData->Unmap();

        for (UIFrameRenderObj* uiFrameRenderObj : _objs) {
            ui::UIFrame* frame = uiFrameRenderObj->frame;
            assert(frame);
            if (!frame->IsShown()) {
                continue;
            }

            UIFrameConstants* frameConstants = uiFrameRenderObj->frameData->Map<UIFrameConstants>();
            frameConstants->borderSize       = uiFrameRenderObj->borderSize;
            frameConstants->borderColor      = uiFrameRenderObj->borderColor;
            frameConstants->bgColor = uiFrameRenderObj->bgColor;
            uiFrameRenderObj->frameData->Unmap();

            renderQueue->AddDrawItem(1, uiFrameRenderObj->item);
        }
    }
};
