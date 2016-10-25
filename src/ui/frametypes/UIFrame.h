#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include "ScriptHandler.h"
#include "RenderObj.h"
#include "Viewport.h"

namespace ui {
class ScriptApi;


enum class FrameType : uint32_t {
    UIFRAME = 0,
    EDITBOX,
    LABEL,
    TEXTLIST,
    COUNT,
};

// positions are rendered based on this for now until i get relative positioning n stuff
const uint32_t INTERNAL_UI_RESOLUTION_HEIGHT{ 600 };
const uint32_t INTERNAL_UI_RESOLUTION_WIDTH{ 800 };

struct FrameScale {
    float x;
    float y;
    float z;
    uint32_t width, height;
    glm::vec3 rot;
};

class UIFrame {
public:
    struct UIFrameDesc {
        UIFrameDesc() : name(""), x(0), y(0), z(0), rot(0.f, 0.f, 0.f),
            width(0), height(0), show(true), acceptMouse(false), parent(0) {}

        std::string name;
        float x;
        float y;
        float z;
        
        glm::vec3 rot;

        uint32_t width;
        uint32_t height;
        bool show;
        bool acceptMouse;
        UIFrame* parent{ nullptr };
    };

protected:
    UIFrameDesc m_frameDesc;
    FrameType m_frameType;
    BaseScriptHandler* m_scriptHandler;

public:
    UIFrame(UIFrameDesc frameDesc) : m_frameDesc(frameDesc), m_frameType(FrameType::UIFRAME) {}
    UIFrame(UIFrameDesc frameDesc, BaseScriptHandler* scriptHandler)
        : m_frameDesc(frameDesc), m_frameType(FrameType::UIFRAME), m_scriptHandler(scriptHandler) {}
    const UIFrameDesc& GetFrameDesc() const { return m_frameDesc; }
    FrameType GetFrameType() const { return m_frameType; }
    void Show() { m_frameDesc.show = true; }
    void Hide() { m_frameDesc.show = false; }
    bool IsShown() const { return m_frameDesc.show; }
    std::string GetFrameName() const { return m_frameDesc.name; }
    // Returns true if wants to be shown and all parent's are
    bool IsVisible() const {
        if (!m_frameDesc.show)
            return false;
        if (m_frameDesc.parent) {
            return m_frameDesc.parent->IsVisible();
        }
        else
            return m_frameDesc.show;
    };

    FrameScale GetScaledSize(Viewport viewport) const {
        FrameScale scaledFrame;
        scaledFrame.width =
            static_cast<uint32_t>(((float)m_frameDesc.width / INTERNAL_UI_RESOLUTION_WIDTH) * viewport.width);
        scaledFrame.height =
            static_cast<uint32_t>(((float)m_frameDesc.height / INTERNAL_UI_RESOLUTION_HEIGHT) * viewport.height);

        scaledFrame.x = (m_frameDesc.x / INTERNAL_UI_RESOLUTION_WIDTH) * viewport.width;
        scaledFrame.y = (m_frameDesc.y / INTERNAL_UI_RESOLUTION_HEIGHT) * viewport.height;
        scaledFrame.z = (m_frameDesc.z);
        scaledFrame.rot = m_frameDesc.rot;
        return scaledFrame;
    }

    UIFrame* GetParent() const { return m_frameDesc.parent; }
    void InitializeScriptHandler(ScriptApi* scriptApi) {
        if (m_scriptHandler) {
            m_scriptHandler->Initialize(scriptApi);
            m_scriptHandler->OnLoad(*this);
        }
    }
    virtual void OnClick() {}
    virtual void DoUpdate(float ms) {}
};
}
