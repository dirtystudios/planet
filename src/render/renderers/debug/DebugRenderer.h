#pragma once

#include <glm/gtx/hash.hpp>
#include <memory>
#include <vector>
#include "DebugDrawInterface.h"
#include "DrawItem.h"
#include "LRUCache.h"
#include "MeshGeometry.h"
#include "Pool.h"
#include "Rectangle.h"
#include "Renderer.h"
#include "StateGroup.h"

struct DebugVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 color;

    static const gfx::VertexLayoutDesc vertexLayoutDesc() {
        static gfx::VertexLayoutDesc vld = {{
            {gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
            {gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float},
            {gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Color0, gfx::VertexAttributeStorage::Float},
        }};
        return vld;
    }
};

struct DebugViewConstants {
    glm::mat4 view;
    glm::mat4 proj;
};

class DebugRenderer : public Renderer, public DebugDrawInterface {
private:
    using SphereVertexCache = LRUCache<size_t, std::vector<DebugVertex>*>;
    using DebugVertexVec    = std::vector<DebugVertex>;
    struct {
        DebugVertexVec wireframe2D;
        DebugVertexVec wireframe3D;
        DebugVertexVec filled2D;
        DebugVertexVec filled3D;
    } _buffers;

    ConstantBuffer*        _3DviewConstants{nullptr};
    ConstantBuffer*        _2DviewConstants{nullptr};
    gfx::BufferId          _vertexBuffer{0};
    const gfx::StateGroup* _2DwireframeSG{nullptr};
    const gfx::StateGroup* _2DfilledSG{nullptr};
    const gfx::StateGroup* _3DwireframeSG{nullptr};
    const gfx::StateGroup* _3DfilledSG{nullptr};
    const gfx::StateGroup* _3DwireframeSphereSG{nullptr};

    std::vector<const gfx::DrawItem*>  _drawItems;
    std::unique_ptr<SphereVertexCache> _sphereCache;

    std::vector<std::pair<glm::mat4, gfx::BufferId>> _3DwireframeSpheres;
    MeshGeometry* _sphereGeometry{nullptr};

public:
    DebugRenderer();
    ~DebugRenderer();

    virtual void AddLine2D(const glm::vec2& start, const glm::vec2& end, glm::vec3 color) final;
    virtual void AddRect2D(const dm::Rect2Df& rect, const glm::vec3& color, bool filled = false) final;
    virtual void AddCircle2D(const glm::vec2& origin, float r, const glm::vec3& color, bool filled = false) final;

    virtual void AddLine3D(const glm::vec3& start, const glm::vec3& end, glm::vec3 color) final;
    virtual void AddRect3D(const dm::Rect3Df& rect, const glm::vec3& color, bool filled = false) final;
    virtual void AddSphere3D(const glm::vec3& origin, float radius) final;
    virtual void AddCube(const dm::BoundingBox& box, const glm::vec3& color, bool filled = false) final;

private:
    virtual void OnInit() final;
    virtual void Submit(RenderQueue* renderQueue, const FrameView* view) final;
};
