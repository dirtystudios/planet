#pragma once

#include <string>
#include <memory>
#include "RenderObj.h"
#include "StateGroup.h"
#include "Rectangle.h"
#include "ConstantBuffer.h"

class DebugRenderer;

enum class DebugRenderObjType : uint32_t {
    Rect2D = 0,
};

class DebugRenderObj : public RenderObj {
private:
    friend DebugRenderer;
    DebugRenderObjType m_type;
    std::unique_ptr<const gfx::DrawItem> m_drawItem;
public:
    DebugRenderObj(DebugRenderObjType type)
        : RenderObj(RendererType::Debug), m_type(type){}
};

class DebugRect2DRenderObj : public DebugRenderObj {
private:
    friend DebugRenderer;

private:
    Rectangle          m_rect;
    glm::vec3          m_color;

    ConstantBuffer*    m_constantBuffer{ nullptr };
    size_t             m_vertOffset{ 0 };
    
    const gfx::StateGroup* _group{ nullptr };
    const gfx::StateGroup* _cursorGroup{ nullptr };

public:
    DebugRect2DRenderObj(const Rectangle& rect, const glm::vec3& color)
        : DebugRenderObj(DebugRenderObjType::Rect2D), m_rect(rect), m_color(color) {}

    void rect(const Rectangle& rect) { m_rect = rect; };
    void color(const glm::vec3& color) { m_color = color; };
};