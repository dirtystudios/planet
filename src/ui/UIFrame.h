#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include "ScriptHandler.h"
#include "RenderObj.h"
#include "Viewport.h"

namespace ui {

enum class FrameType : uint32_t {
    UIFRAME = 0,
    EDITBOX,
    LABEL,
    COUNT,
};

// positions are rendered based on this for now until i get relative positioning n stuff
const uint32_t INTERNAL_UI_RESOLUTION_HEIGHT{ 600 };
const uint32_t INTERNAL_UI_RESOLUTION_WIDTH{ 800 };

struct FrameScale {
    float x;
    float y;
    uint32_t width, height;
};

class UIFrame {
public:
    struct UIFrameDesc {
        UIFrameDesc() : name(""), x(0), y(0), width(0), height(0), shown(true), acceptMouse(false), parent(0) {}

        std::string name;
        float x;
        float y;
        uint32_t width;
        uint32_t height;
        bool shown;
        bool acceptMouse;
        UIFrame* parent{ nullptr };
    };

protected:
    UIFrameDesc m_frameDesc;
    FrameType m_frameType;
    ScriptHandler* m_scriptHandler;

public:
    UIFrame(UIFrameDesc frameDesc) : m_frameDesc(frameDesc), m_frameType(FrameType::UIFRAME) {};
    UIFrame(UIFrameDesc frameDesc, ScriptHandler* scriptHandler)
        : m_frameDesc(frameDesc), m_frameType(FrameType::UIFRAME), m_scriptHandler(scriptHandler) {};
    UIFrameDesc* GetFrameDesc() { return &m_frameDesc; };
    FrameType GetFrameType() { return m_frameType; };
    void Show() { m_frameDesc.shown = true; };
    void Hide() { m_frameDesc.shown = false; };
    // Returns true if wants to be shown and all parent's are
    bool IsShown() {
        if (!m_frameDesc.shown)
            return false;
        if (m_frameDesc.parent) {
            return m_frameDesc.parent->IsShown();
        }
        else
            return m_frameDesc.shown;
    };

    FrameScale GetScaledSize(Viewport viewport) {
        FrameScale scaledFrame;
        scaledFrame.width =
            static_cast<uint32_t>(((float)m_frameDesc.width / INTERNAL_UI_RESOLUTION_WIDTH) * viewport.width);
        scaledFrame.height =
            static_cast<uint32_t>(((float)m_frameDesc.height / INTERNAL_UI_RESOLUTION_HEIGHT) * viewport.height);

        scaledFrame.x = (m_frameDesc.x / INTERNAL_UI_RESOLUTION_WIDTH) * viewport.width;
        scaledFrame.y = (m_frameDesc.y / INTERNAL_UI_RESOLUTION_HEIGHT) * viewport.height;
        return scaledFrame;
    }

    UIFrame* GetParent() { return m_frameDesc.parent; };
    virtual void OnClick() {};
    virtual void DoUpdate(float ms) {};
};
}
