#pragma once
#include "RenderDevice.h"
#include "Config.h"
#include "Helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "UIFrame.h"

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

    void* operator new (size_t i) {
        return _mm_malloc(i, 16);
    }

    void operator delete (void* p) {
        _mm_free(p);
    }
};

class UIRenderer : public Renderer {
private:
    static constexpr size_t kDefaultVertexBufferSize = sizeof(FrameVertex) * 1024;

    gfx::PipelineStateId _defaultPS;
    gfx::BufferId _vertexBuffer;
    size_t _bufferOffset;
    size_t _bufferSize;
    ConstantBuffer* _viewData{nullptr};

    std::vector<UIFrameRenderObj*> _objs;

public:
    void OnInit() {
        gfx::PipelineStateDesc psd;
        psd.vertexShader               = GetShaderCache()->Get(gfx::ShaderType::VertexShader, "ui");
        psd.pixelShader                = GetShaderCache()->Get(gfx::ShaderType::PixelShader, "ui");
        gfx::VertexLayoutDesc vld = {{{
            gfx::VertexAttributeType::Float4, gfx::VertexAttributeUsage::Position,
        }}};
        psd.vertexLayout            = GetVertexLayoutCache()->Get(vld);
        psd.topology                = gfx::PrimitiveType::Triangles;
        psd.blendState.enable       = true;
        psd.blendState.srcRgbFunc   = gfx::BlendFunc::SrcAlpha;
        psd.blendState.srcAlphaFunc = psd.blendState.srcRgbFunc;
        psd.blendState.dstRgbFunc   = gfx::BlendFunc::OneMinusSrcAlpha;
        psd.blendState.dstAlphaFunc = psd.blendState.dstRgbFunc;

        _defaultPS = GetPipelineStateCache()->Get(psd);
        _viewData  = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIViewConstants));

        _vertexBuffer = GetRenderDevice()->AllocateBuffer(gfx::BufferType::VertexBuffer, kDefaultVertexBufferSize,
                                                          gfx::BufferUsage::Static);
        _bufferOffset = 0;
        _bufferSize   = kDefaultVertexBufferSize;

        assert(_vertexBuffer);
        assert(_defaultPS);
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
    RenderObj* RegisterFrame(ui::UIFrame* frame) {
        UIFrameRenderObj* uiRenderObj = new UIFrameRenderObj();
        uiRenderObj->frame            = frame;

        // currently frame location is immutable after registration
        float w    = static_cast<float>(frame->GetFrameDesc()->width);
        float h    = static_cast<float>(frame->GetFrameDesc()->height);
        float xpos = frame->GetFrameDesc()->x;
        float ypos = frame->GetFrameDesc()->y;

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
        uint8_t* mapped = GetRenderDevice()->MapMemory(_vertexBuffer, gfx::BufferAccess::Write);
        assert(mapped);
        memcpy(&mapped[_bufferOffset], &vertices, verticesSize);
        GetRenderDevice()->UnmapMemory(_vertexBuffer);
        _bufferOffset += verticesSize;

        uiRenderObj->frameData = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIFrameConstants));

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
            frameConstants->bgColor = uiFrameRenderObj->borderColor;
            uiFrameRenderObj->frameData->Unmap();

            gfx::DrawItemDesc did;
            did.drawCall      = uiFrameRenderObj->drawCall;
            did.pipelineState = _defaultPS;
            did.streamCount   = 1;
            did.streams[0]    = uiFrameRenderObj->stream;
            did.bindingCount  = 2;
            did.bindings[0]   = _viewData->GetBinding(1);
            did.bindings[1]   = uiFrameRenderObj->frameData->GetBinding(2);

            const gfx::DrawItem* item = GetRenderDevice()->GetDrawItemEncoder()->Encode(did);
            renderQueue->AddDrawItem(1, item);
        }
    }
};
