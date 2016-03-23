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
        UIRenderer *m_uiRenderer;
        TextRenderer* m_textRenderer;
        // should probly use a tree for this, oops
        // This stores <Parent, Child>
        std::unordered_multimap<UIFrame*, UIFrame*> m_frameTree;
        std::vector<UIFrame*> m_uiFrames;
        std::vector<UIFrame*> m_prevShownUiFrames;
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
		
		bool HandleMouseX(float xPos);
		bool HandleMouseY(float yPos);
		bool HandleMouse1(float value);

    private:
        void RenderFrame(UIFrame* uiFrame);

        void PreProcess();
        void PostProcess(float ms);
        void ProcessFrames();
        //void HandleKeyBoardPress(
        
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

        void RenderChildren(UIFrame* frame, std::unordered_multimap<UIFrame*, UIFrame*> &map) {
            if (!frame->IsShown())
                return;
            RenderFrame(frame);

            auto its = map.equal_range(frame);
            for (auto it = its.first; it != its.second; ++it) {
                RenderChildren(it->second, map);
            }
        }
    };
}