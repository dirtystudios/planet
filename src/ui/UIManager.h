#pragma once
#include "InputManager.h"
#include "UIFrame.h"
#include "EditBox.h"
#include "RenderDevice.h"
#include "UIRenderer.h"
#include "TextRenderer.h"
#include "KeyboardManager.h"
#include <unordered_map>
#include <vector>

namespace ui {
    class UIManager {
    private:
        // positions are rendered based on this for now until i get relative positioning n stuff
        const uint32_t INTERNAL_UI_RESOLUTION_HEIGHT { 600 };
        const uint32_t INTERNAL_UI_RESOLUTION_WIDTH { 800 };

        struct FrameScale {
            float x;
            float y;
            uint32_t width, height;
        };
        

        UIRenderer *m_uiRenderer;
        TextRenderer* m_textRenderer;
        // should probly use a tree for this, oops
        // This stores <Parent, Child>
        std::unordered_multimap<UIFrame*, UIFrame*> m_frameTree;
        std::vector<UIFrame*> m_uiFrames;
        uint32_t m_windowWidth, m_windowHeight;
        // only 1 thing should have focus at a time, so these can go here
        float m_cursorBlink = 0;
        bool m_drawCaret = false;
        EditBox* m_focusedEditBox = 0;
        input::KeyboardManager* m_keyboardManager;
		input::InputContext* m_uiInputContext;
		bool m_mouseDown = false;
		float m_mouseX=0.f, m_mouseY=0.f;
    public:
        UIManager(input::KeyboardManager* keyboardManager, input::InputContext* inputContext, graphics::RenderDevice* renderDevice, uint32_t windowWidth, uint32_t windowHeight)
            : m_windowWidth(windowWidth), m_windowHeight(windowHeight), m_keyboardManager(keyboardManager), m_uiInputContext(inputContext) {
            m_uiRenderer = new UIRenderer(renderDevice, (float)windowWidth, (float)windowHeight);

			m_uiInputContext->BindContext<input::ContextBindingType::Axis>("MousePosX", BIND_MEM_CB(&UIManager::HandleMouseX, this));
			m_uiInputContext->BindContext<input::ContextBindingType::Axis>("MousePosY", BIND_MEM_CB(&UIManager::HandleMouseY, this));
			m_uiInputContext->BindContext<input::ContextBindingType::Action>("MouseKey1", BIND_MEM_CB(&UIManager::HandleMouse1, this));
            
        };
        ~UIManager() {
            delete m_uiRenderer;
        }

        void AddFrame(UIFrame* uiFrame);
        void DoUpdate(float ms);

        // hackish for now
        void SetTextRenderer(TextRenderer* textRenderer) { m_textRenderer = textRenderer; };
        void SetRenderWindowSize(uint32_t width, uint32_t height) {
            m_windowWidth = width;
            m_windowHeight = height;
            m_uiRenderer->SetRenderWindowSize(m_windowWidth, m_windowHeight);
        }

        // Callbacks from inputmanager

		bool HandleMouseX(float xPos);
		bool HandleMouseY(float yPos);
		bool HandleMouse1(float value);

    private:
        void RenderFrame(UIFrame* uiFrame);

        void PreProcess();
        void PostProcess(float ms);
        void ProcessFrames();

        void RenderChildren(UIFrame* frame, std::unordered_multimap<UIFrame*, UIFrame*> &map);
        
        FrameScale GetScaledFrame(UIFrame::UIFrameDesc* frameDesc) {
            FrameScale scaledFrame;
            scaledFrame.width = ((float)frameDesc->width / INTERNAL_UI_RESOLUTION_WIDTH) * m_windowWidth;
            scaledFrame.height = ((float)frameDesc->height / INTERNAL_UI_RESOLUTION_HEIGHT) * m_windowHeight;

            scaledFrame.x = (frameDesc->x / INTERNAL_UI_RESOLUTION_WIDTH) * m_windowWidth;
            scaledFrame.y = (frameDesc->y / INTERNAL_UI_RESOLUTION_HEIGHT) * m_windowHeight;
            return scaledFrame;
        }

        template <typename T, typename T2>
        void RemoveChildren(T *frame, std::unordered_multimap<T2, T2> &map) {
            bool hasChildren = false;
            auto its = map.equal_range(frame);
            for (auto it = its.first; it != its.second; ++it) {
                RemoveChildren(it->second, map);
                hasChildren = true;
            }
            if (hasChildren)
                map.erase(frame);
        }
    };
}