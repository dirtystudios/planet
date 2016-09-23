#include "DebugRenderer.h"
#include "Log.h"
#include "StateGroupEncoder.h"
#include "DrawItemEncoder.h"

#include "glm/gtc/matrix_transform.hpp"

struct DebugViewConstants {
    glm::mat4 projection;
};

struct DebugConstants {
    glm::vec3 color;
};

struct Rect2DVertex {
    glm::vec2 pos;
};

static constexpr size_t kDefaultVertexBufferSize = sizeof(Rect2DVertex) * 48;

void DebugRenderer::OnInit() {
    _viewData = GetConstantBufferManager()->GetConstantBuffer(sizeof(DebugViewConstants));
    gfx::BufferDesc desc =
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize);
    _rectVertexBuffer = GetRenderDevice()->AllocateBuffer(desc);
    _rectBufferOffset = 0;

    gfx::BlendState blendState;
    blendState.enable = false;

    gfx::RasterState rasterState;
    rasterState.fillMode = gfx::FillMode::Wireframe;
    rasterState.cullMode = gfx::CullMode::None;
    rasterState.windingOrder = gfx::WindingOrder::FrontCCW;

    gfx::DepthState depthState;
    depthState.enable = false;

    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "debug"));
    encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "debug"));
    gfx::VertexLayoutDesc vld = { {
        { gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float },
        } };
    encoder.SetVertexLayout(GetVertexLayoutCache()->Get(vld));
    encoder.SetBlendState(blendState);
    encoder.SetDepthState(depthState);
    encoder.SetRasterState(rasterState);
    encoder.SetPrimitiveType(gfx::PrimitiveType::LineStrip);
    encoder.BindResource(_viewData->GetBinding(1));
    encoder.SetVertexBuffer(_rectVertexBuffer);
    _base = encoder.End();
    assert(_rectVertexBuffer);
    assert(_viewData);
}

void DebugRenderer::Register(DebugRenderObj* debugRO) {
    switch (debugRO->m_type) {
    case DebugRenderObjType::Rect2D:
        RegisterRect2D(static_cast<DebugRect2DRenderObj*>(debugRO));
        break;
    default:
        LOG_E("[DebugRenderer] Unsupported DebugRenderObjType %d", debugRO->m_type);
        break;
    }
    return;
}

void DebugRenderer::RegisterRect2D(DebugRect2DRenderObj* /*get*/ rekt) {

    rekt->m_constantBuffer = GetConstantBufferManager()->GetConstantBuffer(sizeof(DebugConstants));

    gfx::StateGroupEncoder encoder;
    encoder.Begin(_base);
    encoder.BindResource(rekt->m_constantBuffer->GetBinding(2));
    rekt->_group = encoder.End();


    assert(_rectBufferOffset + (5 * sizeof(Rect2DVertex)) < kDefaultVertexBufferSize);
    _rectBufferOffset += (5 * sizeof(Rect2DVertex));

    _objs.push_back(rekt);
}

void DebugRenderer::Unregister(DebugRenderObj* renderObj) {
    auto it =
        std::find_if(begin(_objs), end(_objs), [&renderObj](DebugRenderObj* obj) { return obj == renderObj; });
    if (it != _objs.end()) {
        _objs.erase(it);
    }
}

void DebugRenderer::SetRect2DVertices(DebugRect2DRenderObj* debugRenderObj) {
    size_t bufferOffset = debugRenderObj->m_vertOffset * sizeof(Rect2DVertex);
    assert(bufferOffset + (5 * sizeof(Rect2DVertex)) < kDefaultVertexBufferSize);

    uint8_t* mapped = GetRenderDevice()->MapMemory(_rectVertexBuffer, gfx::BufferAccess::WriteNoOverwrite);
    assert(mapped);
    Rect2DVertex* vertices = reinterpret_cast<Rect2DVertex*>(mapped + bufferOffset);
    Rectangle rect = debugRenderObj->m_rect;

    vertices[0] = { {rect.bl.x, rect.bl.y} };
    vertices[1] = { {rect.tr.x, rect.bl.y} };
    vertices[2] = { {rect.tr.x, rect.tr.y} };
    vertices[3] = { {rect.bl.x, rect.tr.y} };
    vertices[4] = { {rect.bl.x, rect.bl.y} };

    GetRenderDevice()->UnmapMemory(_rectVertexBuffer);
}

const gfx::DrawItem* DebugRenderer::CreateRect2DDrawItem(DebugRect2DRenderObj* renderObj) {
    // 'loop' around the vertexBuffer, this is currently to fix directx, and as this buffer fills, 
    //  im sure it's going to break again
    if (((sizeof(Rect2DVertex) *_rectBufferOffset )+ 5 * sizeof(Rect2DVertex)) >= kDefaultVertexBufferSize) {
        assert(_rectBufferOffset != 0);
        _rectBufferOffset = 0;
    }

    renderObj->m_vertOffset = _rectBufferOffset;

    SetRect2DVertices(renderObj);
    _rectBufferOffset += renderObj->m_vertOffset;

    gfx::DrawCall drawCall;
    drawCall.type = gfx::DrawCall::Type::Arrays;
    drawCall.primitiveCount = 5;
    drawCall.offset = renderObj->m_vertOffset;

    return gfx::DrawItemEncoder::Encode(GetRenderDevice(), drawCall, &renderObj->_group, 1);
}

void DebugRenderer::Submit(RenderQueue* queue, RenderView* view) {
    _viewData->Map<DebugViewConstants>()->projection = glm::ortho(0.0f, view->viewport->width, 0.0f, view->viewport->height); // TODO: this should be set by renderView
    _viewData->Unmap();

    for (auto& obj : _objs) {
        switch (obj->m_type) {
        case DebugRenderObjType::Rect2D: {
            DebugRect2DRenderObj* rectRO = static_cast<DebugRect2DRenderObj*>(obj);
            rectRO->m_constantBuffer->Map<DebugConstants>()->color = rectRO->m_color;
            rectRO->m_constantBuffer->Unmap();
            obj->m_drawItem.reset(CreateRect2DDrawItem(rectRO));
            queue->AddDrawItem(3, obj->m_drawItem.get());
            break;
        }
        default:
            LOG_E("[DebugRenderer] Invalid type in submit, %d", obj->m_type);
            break;
        }
    }
}