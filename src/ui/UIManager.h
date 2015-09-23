#pragma once
#include "InputContext.h"
#include "UIFrame.h"
#include "RenderDevice.h"
#include "UIRenderer.h"

#include <vector>

namespace ui {
    class UIManager {
    private:
        input::InputContext *m_inputContext;
        UIRenderer *m_uiRenderer;
        std::vector<UIFrame*> m_uiFrames;
        std::vector<UIFrame*> m_prevShownUiFrames;
        uint32_t m_windowWidth, m_windowHeight;
    public:
        UIManager(input::InputContext* context, graphics::RenderDevice* renderDevice, uint32_t windowWidth, uint32_t windowHeight)
            : m_inputContext(context), m_windowWidth(windowWidth), m_windowHeight(windowHeight) {
            m_uiRenderer = new UIRenderer(renderDevice, (float)windowWidth, (float)windowHeight);
        };
        ~UIManager() {
            delete m_uiRenderer;
        }

        void AddFrame(UIFrame* uiFrame);
        void DoUpdate(float dt);
    };
}