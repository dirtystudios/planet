#include "UIRenderer.h"

struct UIViewConstants {
    glm::mat4 projection;
    glm::mat4 view;
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
    _viewData            = services()->constantBufferManager()->GetConstantBuffer(sizeof(UIViewConstants), "ui2DViewConstants");
    _viewData3D          = services()->constantBufferManager()->GetConstantBuffer(sizeof(UIViewConstants), "ui3DViewConstants");
    gfx::BufferDesc desc = 
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize, "uiVB");
    _vertexBuffer        = device()->AllocateBuffer(desc);
    _bufferOffset        = 0;
    _bufferSize          = kDefaultVertexBufferSize;

    std::vector<uint32_t> whiteImageData(16 * 16, 0xFFFFFFFF);
    _whiteTex = device()->CreateTexture2D(gfx::PixelFormat::RGBA8Unorm, 16, 16, whiteImageData.data(), "uiRendererWhiteTex");

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
    encoder.BindResource(_viewData->GetBinding(1));
    const gfx::StateGroup* bind2D = encoder.End();

    encoder.Begin();
    encoder.BindResource(_viewData3D->GetBinding(1));
    const gfx::StateGroup* bind3D = encoder.End();

    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "ui"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "ui"));
    gfx::VertexLayoutDesc vld = {{{gfx::VertexAttributeType::Float4, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
                                  {gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float}}};
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Get(vld));
    encoder.SetBlendState(blendState);
    encoder.SetDepthState(depthState);
    encoder.SetRasterState(rasterState);
    encoder.SetVertexBuffer(_vertexBuffer);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Triangles);
    const gfx::StateGroup* baseBind = encoder.End();

    _base = gfx::StateGroupEncoder::Merge({ bind2D, baseBind });
    _base3D = gfx::StateGroupEncoder::Merge({ bind3D, baseBind });

    assert(_vertexBuffer);
    assert(_viewData);

    delete bind2D;
    delete bind3D;
    delete baseBind;
}

UIRenderer::~UIRenderer() {
    // TODO
}

void UIRenderer::Register(UIFrameRenderObj* uiRenderObj) {
    dg_assert_nm(uiRenderObj != nullptr);
    dg_assert(std::find(begin(_objs), end(_objs), uiRenderObj) == end(_objs), "UIFrameRenderObj already registered");

    // load image 
    if (uiRenderObj->_texId == 0 && uiRenderObj->_texPath != "") {
        dimg::Image imageData;
        if (!LoadImageFromFile(uiRenderObj->_texPath.c_str(), &imageData)) {
            LOG_E("Failed to load image: %s", uiRenderObj->_texPath.c_str());
        }

        uiRenderObj->_texId = device()->CreateTexture2D(imageData.pixelFormat == dimg::PixelFormat::RGBA8Unorm ? gfx::PixelFormat::RGBA8Unorm : gfx::PixelFormat::RGB8Unorm, 
            imageData.width, imageData.height, imageData.data, "uiTex-"+ fs::FileName(uiRenderObj->_texPath.c_str()));
    }

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
    uiRenderObj->_drawCall.startOffset         = _bufferOffset / sizeof(FrameVertex);

    // pack all frames into a single buffer
    uint8_t* mapped = device()->MapMemory(_vertexBuffer, gfx::BufferAccess::WriteNoOverwrite);
    assert(mapped);
    memcpy(&mapped[_bufferOffset], &vertices, verticesSize);
    device()->UnmapMemory(_vertexBuffer);
    _bufferOffset += verticesSize;

    uiRenderObj->_frameData = services()->constantBufferManager()->GetConstantBuffer(sizeof(UIFrameConstants), "uiFrameConstants");

    gfx::StateGroupEncoder encoder;
    if (uiRenderObj->_usePerspective)
        encoder.Begin(_base3D);
    else
        encoder.Begin(_base);
    encoder.BindResource(uiRenderObj->_frameData->GetBinding(2));
    if (uiRenderObj->_texId == 0)
        encoder.BindTexture(0, _whiteTex, gfx::ShaderStageFlags::PixelBit);
    else {
        uiRenderObj->_bgColor = { 1.f, 1.f, 1.f, 1.f };
        encoder.BindTexture(0, uiRenderObj->_texId, gfx::ShaderStageFlags::PixelBit);
    }
    uiRenderObj->_group = encoder.End();

    uiRenderObj->_item = gfx::DrawItemEncoder::Encode(device(), uiRenderObj->_drawCall, &uiRenderObj->_group, 1);

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

void UIRenderer::Submit(RenderQueue* renderQueue, const FrameView* view) {
    glm::mat4 projection = view->ortho;

    UIViewConstants* viewConstants2d = _viewData->Map<UIViewConstants>();
    viewConstants2d->projection = projection;
    viewConstants2d->view = glm::mat4();
    _viewData->Unmap();

    UIViewConstants* viewConstants3d = _viewData3D->Map<UIViewConstants>();
    viewConstants3d->view = view->view;
    viewConstants3d->projection = view->projection;
    _viewData3D->Unmap();

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
