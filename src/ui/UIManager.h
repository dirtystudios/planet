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

#include "SimulationManager.h"
#include "ComponentManager.h"
#include "EventManager.h"
#include "InputEvent.h"

#include <memory>
#include <map>
#include <vector>

namespace ui {
class UIManager : public ComponentManager {
private:
    std::vector<std::unique_ptr<UIDomTree>> m_domTrees;
    std::map<uint64_t, std::vector<UIFrame*>> m_uiFrames;

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
    input::InputContext*    m_debugContext;
    bool                    m_mouseDown = false;
    bool                    m_mouse2Down = false;
    float                   m_mouseX = 0.f, m_mouseY = 0.f;
    ScriptApi               m_scriptApi;

public:
    UIManager(EventManager* em, input::KeyboardManager* keyboardManager, input::InputContext* debugContext, Viewport viewport, 
        TextRenderer* textRenderer, UIRenderer* uiRenderer, DebugDrawInterface* debug)
        : m_viewport(viewport), m_keyboardManager(keyboardManager), m_debugContext(debugContext), m_scriptApi(this),
          m_textRenderer(textRenderer), m_uiRenderer(uiRenderer), m_debugRenderer(debug) {

        em->subscribe<InputEvent>(std::bind(&UIManager::HandleInputEvent, this, std::placeholders::_1));
        
        m_debugContext->BindContext<input::ContextBindingType::Action>("debug5", std::bind(&UIManager::ToggleDebugDraw, this, std::placeholders::_1));
        config::ConsoleCommands::getInstance().RegisterCommand("toggleFocusDebug", std::bind(&UIManager::ToggleDebugDrawConsole, this, std::placeholders::_1));
    };
    ~UIManager() {}

    void UpdateViewport(const Viewport& vp) override;

    void DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) override;

    UIFrame* GetFrame(const std::string& name);

    bool HandleInputEvent(const InputEvent& ev);

    // Callbacks from inputmanager

    bool ToggleDebugDraw(const input::InputContextCallbackArgs& args) { m_debugDrawFocus = !m_debugDrawFocus; return false; }
    std::string ToggleDebugDrawConsole(const std::vector<std::string>&) { m_debugDrawFocus = !m_debugDrawFocus; return "Editbox Focus debug " + std::string(m_debugDrawFocus ? "on" : "off"); }

private:
    bool HandleMouseX(const input::InputContextCallbackArgs& args);
    bool HandleMouseY(const input::InputContextCallbackArgs& args);
    bool HandleMouse1(const input::InputContextCallbackArgs& args);
    bool HandleMouse2(const input::InputContextCallbackArgs& args);

    void AddFrameObj(uint64_t key, UI* ui, Spatial* spatial);

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
