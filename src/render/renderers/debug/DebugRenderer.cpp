#include "DebugRenderer.h"
#include "DMath.h"
#include "DrawItemEncoder.h"
#include "MeshGeneration.h"
#include "StateGroupDecoder.h"
#include "StateGroupEncoder.h"
#include "glm/gtc/matrix_transform.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace dm;

constexpr float  kLineWidth               = 1.f;
constexpr float  kHalfLineWidth           = kLineWidth / 2.f;
constexpr size_t kDefaultVertexBufferSize = sizeof(DebugVertex) * 4096;

static glm::vec3 to3(const glm::vec2& v) { return {v.x, v.y, 0}; }
static glm::vec4 to4(const glm::vec3& v) { return {v.x, v.y, v.z, 1}; }

DebugRenderer::DebugRenderer() : Renderer(RendererType::Debug) {}

void DebugRenderer::OnInit() {
    MeshGeometryData geometryData;
    dgen::GenerateIcoSphere(3, &geometryData);
    _sphereGeometry = new MeshGeometry(device(), { geometryData });

    _sphereCache.reset(new SphereVertexCache(16, [&](const size_t& key, std::vector<DebugVertex>*& value) { delete value; }));
    _3DviewConstants = services()->constantBufferManager()->GetConstantBuffer(sizeof(DebugViewConstants), "debug3Dview");
    _2DviewConstants = services()->constantBufferManager()->GetConstantBuffer(sizeof(DebugViewConstants), "debug2Dview");

    gfx::BufferDesc desc = 
        gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize, "debugVB");
    _vertexBuffer        = device()->AllocateBuffer(desc);

    gfx::BlendState blendState;
    blendState.enable = false;

    gfx::RasterState rasterState;
    rasterState.fillMode     = gfx::FillMode::Wireframe;
    rasterState.cullMode     = gfx::CullMode::None;
    rasterState.windingOrder = gfx::WindingOrder::FrontCCW;

    gfx::DepthState depthState;
    depthState.enable = false;

    gfx::StateGroupEncoder encoder;

    encoder.Begin();
    encoder.BindResource(_2DviewConstants->GetBinding(0));
    const gfx::StateGroup* bind2D = encoder.End();

    encoder.Begin();
    encoder.BindResource(_3DviewConstants->GetBinding(0));
    const gfx::StateGroup* bind3D = encoder.End();

    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "debug"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "debug"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Get(DebugVertex::vertexLayoutDesc()));
    encoder.SetBlendState(blendState);
    encoder.SetDepthState(depthState);
    encoder.SetRasterState(rasterState);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Lines);
    encoder.SetVertexBuffer(_vertexBuffer);
    const gfx::StateGroup* wireframeSG = encoder.End();

    encoder.Begin(wireframeSG);
    rasterState.fillMode = gfx::FillMode::Solid;
    encoder.SetRasterState(rasterState);
    encoder.SetPrimitiveType(gfx::PrimitiveType::Triangles);
    const gfx::StateGroup* filledSG = encoder.End();

    _2DfilledSG          = gfx::StateGroupEncoder::Merge({bind2D, filledSG});
    _2DwireframeSG       = gfx::StateGroupEncoder::Merge({bind2D, wireframeSG});
    _3DfilledSG          = gfx::StateGroupEncoder::Merge({bind3D, filledSG});
    _3DwireframeSG       = gfx::StateGroupEncoder::Merge({bind3D, wireframeSG});
    _3DwireframeSphereSG = gfx::StateGroupEncoder::Merge({bind3D, wireframeSG, _sphereGeometry->stateGroup()});

    gfx::StateGroupDecoder decoder(filledSG);
    decoder.ReadRasterState(&rasterState);
    delete filledSG;
    delete wireframeSG;
    delete bind2D;
    delete bind3D;
}

DebugRenderer::~DebugRenderer() {
    // cleanup spheremesh
}

void DebugRenderer::AddLine2D(const glm::vec2& start, const glm::vec2& end, glm::vec3 color) {
    AddRect2D({{start.x - kHalfLineWidth, start.y}, {end.x + kHalfLineWidth, end.y}}, color, true);
}

void DebugRenderer::AddCircle2D(const glm::vec2& origin, float r, const glm::vec3& color, bool filled) {
    DebugVertexVec& buffer  = filled ? _buffers.filled2D : _buffers.wireframe2D;
    const uint32_t kSamples = 15;
    double dt               = 2.0 * M_PI / (double)kSamples;
    glm::vec3 o             = to3(origin);
    for (uint32_t i = 0; i < kSamples; ++i) {
        buffer.push_back({o, {0, 0}, color});

        glm::vec3 p1 = {r * cos(i * dt), r * sin(i * dt), 0};
        glm::vec3 p2 = {r * cos((i + 1) * dt), r * sin((i + 1) * dt), 0};
        p1 += o;
        p2 += o;
        buffer.push_back({p1, {0, 0}, color});
        buffer.push_back({p2, {0, 0}, color});
    }
}

void DebugRenderer::AddSphere3D(const glm::vec3& origin, float radius) {
    //    gfx::BufferId transientBuffer = device()->AllocateBuffer(gfx::BufferDesc::defaultTransient(gfx::BufferUsageFlags::ConstantBufferBit, sizeof(DebugViewConstants)));
    //    dm::Transform transform;
    //    transform.scale(radius);
    //    transform.translate(origin);
    //    _3DwireframeSpheres.emplace_back(transform.matrix(), transientBuffer);
}

void DebugRenderer::AddRect2D(const Rect2Df& rect, const glm::vec3& color, bool filled) {
    DebugVertexVec& buffer = filled ? _buffers.filled2D : _buffers.wireframe2D;
    if (filled) {
        buffer.push_back({to3(rect.bl()), {0, 0}, color});
        buffer.push_back({to3(rect.br()), {0, 0}, color});
        buffer.push_back({to3(rect.tr()), {0, 0}, color});

        buffer.push_back({to3(rect.tr()), {0, 0}, color});
        buffer.push_back({to3(rect.tl()), {0, 0}, color});
        buffer.push_back({to3(rect.bl()), {0, 0}, color});
    }
    else {
        buffer.push_back({ to3(rect.bl()),{ 0, 0 }, color });
        buffer.push_back({ to3(rect.br()),{ 0, 0 }, color });

        buffer.push_back({ to3(rect.br()),{ 0, 0 }, color });
        buffer.push_back({ to3(rect.tr()),{ 0, 0 }, color });

        buffer.push_back({ to3(rect.tr()),{ 0, 0 }, color });
        buffer.push_back({ to3(rect.tl()),{ 0, 0 }, color });

        buffer.push_back({ to3(rect.tl()),{ 0, 0 }, color });
        buffer.push_back({ to3(rect.bl()),{ 0, 0 }, color });
    }
}

void DebugRenderer::AddLine3D(const glm::vec3& start, const glm::vec3& end, glm::vec3 color) {
    //    glm::vec3 dir = end - start;
    //    glm::cross(dir)
}
void DebugRenderer::AddRect3D(const Rect3Df& rect, const glm::vec3& color, bool filled) {
    DebugVertexVec& buffer = filled ? _buffers.filled3D : _buffers.wireframe3D;
    
    if (filled) {
        buffer.push_back({rect.bl(), {0, 0}, color});
        buffer.push_back({rect.br(), {0, 0}, color});
        buffer.push_back({rect.tr(), {0, 0}, color});
        buffer.push_back({rect.tr(), {0, 0}, color});
        buffer.push_back({rect.tl(), {0, 0}, color});
        buffer.push_back({rect.bl(), {0, 0}, color});
    } else {
        buffer.push_back({rect.bl(), {0, 0}, color});
        buffer.push_back({rect.br(), {0, 0}, color});

        buffer.push_back({rect.br(), {0, 0}, color});
        buffer.push_back({rect.tr(), {0, 0}, color});

        buffer.push_back({rect.tr(), {0, 0}, color});
        buffer.push_back({rect.tl(), {0, 0}, color});

        buffer.push_back({rect.tl(), {0, 0}, color});
        buffer.push_back({rect.bl(), {0, 0}, color});
    }
}

void DebugRenderer::Submit(RenderQueue* renderQueue, const FrameView* view) {
    for (const gfx::DrawItem* item : _drawItems) {
        delete item;
    }
    _drawItems.clear();

    gfx::DrawItemEncoder encoder;
    gfx::DrawCall        drawCall;
    drawCall.type   = gfx::DrawCall::Type::Arrays;
    drawCall.startOffset = 0;

    if (_buffers.filled2D.size() + _buffers.wireframe2D.size() > 0) {
        DebugViewConstants* viewConstants = _2DviewConstants->Map<DebugViewConstants>();
        viewConstants->view               = glm::mat4();
        viewConstants->proj               = view->ortho; // TODO: this should be set by renderView
        _2DviewConstants->Unmap();
    }

    if (_buffers.filled3D.size() + _buffers.wireframe3D.size() > 0) {
        DebugViewConstants* viewConstants = _3DviewConstants->Map<DebugViewConstants>();
        viewConstants->view               = view->view;
        viewConstants->proj               = view->projection;;
        _3DviewConstants->Unmap();
    }

    DebugVertex* ptr = reinterpret_cast<DebugVertex*>(device()->MapMemory(_vertexBuffer, gfx::BufferAccess::Write));
    dg_assert_nm(ptr != nullptr);
    {
        if (_buffers.filled2D.size() > 0) {
            drawCall.primitiveCount = _buffers.filled2D.size();

            memcpy(ptr + drawCall.startOffset, _buffers.filled2D.data(), sizeof(DebugVertex) * _buffers.filled2D.size());

            _drawItems.push_back(encoder.Encode(device(), drawCall, {_2DfilledSG}));
            drawCall.startOffset += _buffers.filled2D.size();
            _buffers.filled2D.clear();
        }

        if (_buffers.wireframe2D.size() > 0) {
            drawCall.primitiveCount = _buffers.wireframe2D.size();

            memcpy(ptr + drawCall.startOffset, _buffers.wireframe2D.data(), sizeof(DebugVertex) * _buffers.wireframe2D.size());

            _drawItems.push_back(encoder.Encode(device(), drawCall, {_2DwireframeSG}));
            drawCall.startOffset += _buffers.wireframe2D.size();
            _buffers.wireframe2D.clear();
        }

        if (_buffers.filled3D.size() > 0) {
            drawCall.primitiveCount = _buffers.filled3D.size();

            memcpy(ptr + drawCall.startOffset, _buffers.filled3D.data(), sizeof(DebugVertex) * _buffers.filled3D.size());

            _drawItems.push_back(encoder.Encode(device(), drawCall, {_3DfilledSG}));
            drawCall.startOffset += _buffers.filled3D.size();
            _buffers.filled3D.clear();
        }

        if (_buffers.wireframe3D.size() > 0) {
            drawCall.primitiveCount = _buffers.wireframe3D.size();

            memcpy(ptr + drawCall.startOffset, _buffers.wireframe3D.data(), sizeof(DebugVertex) * _buffers.wireframe3D.size());

            _drawItems.push_back(encoder.Encode(device(), drawCall, {_3DwireframeSG}));
            drawCall.startOffset += _buffers.wireframe3D.size();
            _buffers.wireframe3D.clear();
        }
    }
    device()->UnmapMemory(_vertexBuffer);

    //    // TODO: instancing instead of this monstrosity
    //    for(std::pair<glm::mat4, gfx::BufferId>& sphereParams : _3DwireframeSpheres) {
    //        DebugViewConstants* viewConstants = reinterpret_cast<DebugViewConstants*>(device()->MapMemory(sphereParams.second, gfx::BufferAccess::Write));
    //        viewConstants->view               = renderView->camera->BuildView() * sphereParams.first; // nice hack bro
    //        viewConstants->proj               = renderView->camera->BuildProjection();
    //        device()->UnmapMemory(sphereParams.second);
    //
    //        _drawItems.push_bacwsk(encoder.Encode(device(), _sphereGeometry->drawCall(), {_3DwireframeSphereSG}));
    //
    //    }

    // TODODODODODO; recycle and clean up transient buffers for spheres

    for (const gfx::DrawItem* item : _drawItems) {
        renderQueue->AddDrawItem(0, item);
    }

    _buffers.filled2D.clear();
    _buffers.filled3D.clear();
    _buffers.wireframe2D.clear();
    _buffers.wireframe3D.clear();
}
