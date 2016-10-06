#include "UIRenderer.h"

struct UIViewConstants {
    glm::mat4 projection;
};

struct UIFrameConstants {
    glm::vec4 bgColor;
    glm::vec4 borderColor;
    glm::vec2 borderSize;
    glm::vec2 pad;
    glm::vec3 position;
};

struct FrameVertex {
    glm::vec4 position;
    glm::vec2 texcoords;
};

static constexpr size_t kDefaultVertexBufferSize = sizeof(FrameVertex) * 48;

void UIRenderer::OnInit() {
    _viewData            = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIViewConstants));
    gfx::BufferDesc desc = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize);
    _vertexBuffer        = GetRenderDevice()->AllocateBuffer(desc);
    _bufferOffset        = 0;
    _bufferSize          = kDefaultVertexBufferSize;

    gfx::BlendState blendState;
    blendState.enable       = true;
    blendState.srcRgbFunc   = gfx::BlendFunc::SrcAlpha;
    blendState.srcAlphaFunc = blendState.srcRgbFunc;
    blendState.dstRgbFunc   = gfx::BlendFunc::OneMinusSrcAlpha;
    blendState.dstAlphaFunc = blendState.dstRgbFunc;

    gfx::RasterState rasterState;
    rasterState.fillMode     = gfx::FillMode::Solid;
    rasterState.cullMode     = gfx::CullMode::Back;
    rasterState.windingOrder = gfx::WindingOrder::FrontCCW;

    gfx::DepthState depthState;
    depthState.enable = false;

    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "ui"));
    encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "ui"));
    gfx::VertexLayoutDesc vld = {{
        {gfx::VertexAttributeType::Float4, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
        {gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float}
    }};
    encoder.SetVertexLayout(GetVertexLayoutCache()->Get(vld));
    encoder.SetBlendState(blendState);
    encoder.SetDepthState(depthState);
    encoder.SetRasterState(rasterState);
    encoder.BindResource(_viewData->GetBinding(1));
    encoder.SetVertexBuffer(_vertexBuffer);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Triangles);
    _base = encoder.End();
    assert(_vertexBuffer);
    assert(_viewData);
}

UIRenderer::~UIRenderer() {
    // TODO
}

void UIRenderer::Register(UIFrameRenderObj* uiRenderObj) {
    dg_assert_nm(uiRenderObj != nullptr);
    dg_assert(std::find(begin(_objs), end(_objs), uiRenderObj) == end(_objs), "UIFrameRenderObj already registered");

    float xpos = 0.f;//uiRenderObj->x();
    float ypos = 0.f;// uiRenderObj->y();
    float zpos = 0.f;// uiRenderObj->z();
    float w    = uiRenderObj->width();
    float h    = uiRenderObj->height();

    FrameVertex vertices[6] = {{{xpos, ypos, zpos, 1.f}, {0.0, 0.0}},
                               {{xpos + w, ypos, zpos, 1.f}, {1.0, 0.0}},
                               {{xpos + w, ypos + h, zpos, 1.f},{1.0, 1.0}},
                               {{xpos + w, ypos + h, zpos, 1.f }, {1.0, 1.0}},
                               {{xpos, ypos + h, zpos, 1.f},{0.0, 1.0}},
                               {{xpos, ypos, zpos, 1.f },{0.0, 0.0}}
    };

    size_t verticesSize = sizeof(vertices);
    assert(_bufferOffset + verticesSize < _bufferSize);
    assert(w != 0 && h != 0);
    glm::vec2 borderWidth = {2.f, 2.f};
    glm::vec2 pixelSize   = {1.f / w, 1.f / h};

    uiRenderObj->_stream.vertexBuffer     = _vertexBuffer;
    uiRenderObj->_stream.offset           = 0;
    uiRenderObj->_stream.stride           = sizeof(FrameVertex);
    uiRenderObj->_bgColor                 = {0.f, 0.f, 0.f, 0.5f};
    uiRenderObj->_borderColor             = {0.2f, 0.2f, 0.2f, 0.8f};
    uiRenderObj->_borderSize              = pixelSize * borderWidth;
    uiRenderObj->_drawCall.type           = gfx::DrawCall::Type::Arrays;
    uiRenderObj->_drawCall.primitiveCount = 6;
    uiRenderObj->_drawCall.offset         = _bufferOffset / sizeof(FrameVertex);

    // pack all frames into a single buffer
    uint8_t* mapped = GetRenderDevice()->MapMemory(_vertexBuffer, gfx::BufferAccess::WriteNoOverwrite);
    assert(mapped);
    memcpy(&mapped[_bufferOffset], &vertices, verticesSize);
    GetRenderDevice()->UnmapMemory(_vertexBuffer);
    _bufferOffset += verticesSize;

    uiRenderObj->_frameData = GetConstantBufferManager()->GetConstantBuffer(sizeof(UIFrameConstants));

    gfx::StateGroupEncoder encoder;
    encoder.Begin(_base);
    encoder.BindResource(uiRenderObj->_frameData->GetBinding(2));
    uiRenderObj->_group = encoder.End();

    uiRenderObj->_item = gfx::DrawItemEncoder::Encode(GetRenderDevice(), uiRenderObj->_drawCall, &uiRenderObj->_group, 1);

    _objs.push_back(uiRenderObj);
}

void UIRenderer::Unregister(UIFrameRenderObj* renderObj) {
    auto it = std::find(begin(_objs), end(_objs), renderObj);
    if (it == _objs.end()) {
        return;
    }

    // TODO: cleanup device stuff
    _objs.erase(it);
}

void UIRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    glm::mat4 projection = glm::ortho(0.0f, renderView->viewport->width, 0.0f, renderView->viewport->height);

    _viewData->Map<UIViewConstants>()->projection = projection;
    _viewData->Unmap();

    for (UIFrameRenderObj* uiRenderObj : _objs) {
        if (!uiRenderObj->isRendered()) {
            continue;
        }

        UIFrameConstants* frameConstants = uiRenderObj->_frameData->Map<UIFrameConstants>();
        frameConstants->bgColor          = uiRenderObj->_bgColor;
        frameConstants->borderColor      = uiRenderObj->_borderColor;
        frameConstants->borderSize       = uiRenderObj->_borderSize;
        frameConstants->position         = { uiRenderObj->_x, uiRenderObj->_y, uiRenderObj->_z};
        uiRenderObj->_frameData->Unmap();

        renderQueue->AddDrawItem(1, uiRenderObj->_item);
    }
}
