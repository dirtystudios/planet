#pragma once
#include "DebugDrawInterface.h"
#include "EditBox.h"
#include "ConsoleCommands.h"
#include "InputManager.h"
#include "KeyboardManager.h"
#include "Label.h"
#include "RenderDevice.h"
#include "RenderEngine.h"
#include "TextRenderer.h"
#include "UI.h"
#include "UIFrame.h"
#include "UIRenderer.h"
#include "Viewport.h"
#include "UIDomTree.h"

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace ui {
class UIManager {
private:
    std::vector<std::unique_ptr<UIDomTree>> m_domTrees;
    std::vector<UIFrame*> m_uiFrames;

    UIRenderer*         m_uiRenderer;
    TextRenderer*       m_textRenderer;
    DebugDrawInterface* m_debugRenderer;
    Viewport            m_viewport;
    // only 1 thing should have focus at a time, so these can go here
    float                   m_cursorBlink    = 0;
    bool                    m_drawCaret      = false;
    bool                    m_debugDrawFocus = true;
    EditBox*                m_focusedEditBox = 0;
    input::KeyboardManager* m_keyboardManager;
    input::InputContext*    m_uiInputContext;
    input::InputContext*    m_debugContext;
    bool                    m_mouseDown = false;
    float                   m_mouseX = 0.f, m_mouseY = 0.f;

public:
    UIManager(input::KeyboardManager* keyboardManager, input::InputContext* inputContext, input::InputContext* debugContext, Viewport viewport)
        : m_viewport(viewport), m_keyboardManager(keyboardManager), m_uiInputContext(inputContext), m_debugContext(debugContext) {

        m_uiInputContext->BindContext<input::ContextBindingType::Axis>("MousePosX", std::bind(&UIManager::HandleMouseX, this, std::placeholders::_1));
        m_uiInputContext->BindContext<input::ContextBindingType::Axis>("MousePosY", std::bind(&UIManager::HandleMouseY, this, std::placeholders::_1));
        m_uiInputContext->BindContext<input::ContextBindingType::Action>("MouseKey1", std::bind(&UIManager::HandleMouse1, this, std::placeholders::_1));
        
        m_debugContext->BindContext<input::ContextBindingType::Action>("debug5", std::bind(&UIManager::ToggleDebugDraw, this, std::placeholders::_1));
        config::ConsoleCommands::getInstance().RegisterCommand("toggleFocusDebug", std::bind(&UIManager::ToggleDebugDrawConsole, this, std::placeholders::_1));
    };
    ~UIManager() {}

    void UpdateViewport(Viewport viewport);

    void AddFrameObj(SimObj* uiFrame);
    void DoUpdate(float ms);

    // hackish for now
    void SetTextRenderer(TextRenderer* textRenderer) { m_textRenderer = textRenderer; };
    void SetUIRenderer(UIRenderer* uiRenderer) { m_uiRenderer = uiRenderer; };
    void SetDebugRenderer(DebugDrawInterface* debug) { m_debugRenderer = debug; };

    // Callbacks from inputmanager

    bool HandleMouseX(const input::InputContextCallbackArgs& args);
    bool HandleMouseY(const input::InputContextCallbackArgs& args);
    bool HandleMouse1(const input::InputContextCallbackArgs& args);

    bool ToggleDebugDraw(const input::InputContextCallbackArgs& args) { m_debugDrawFocus = !m_debugDrawFocus; return false; }
    std::string ToggleDebugDrawConsole(const std::vector<std::string>&) { m_debugDrawFocus = !m_debugDrawFocus; return ""; }

private:
    void PreProcess();
    void PostProcess(float ms);
    void ProcessFrames();

    template <typename T, typename T2> void RemoveChildren(T* frame, std::unordered_multimap<T2, T2>& map) {
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
