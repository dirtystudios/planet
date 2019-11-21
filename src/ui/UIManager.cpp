#include "UIManager.h"
#include "Rectangle.h"
#include "Spatial.h"

namespace ui {

bool UIManager::HandleMouseX(const input::InputContextCallbackArgs& xArgs) {
    // x is fine
    m_mouseX = xArgs.value;
    return m_mouseDown;
}

bool UIManager::HandleMouseY(const input::InputContextCallbackArgs& yArgs) {
    // our y needs to be switched
    m_mouseY = m_viewport.height - yArgs.value;
    return m_mouseDown;
}

bool UIManager::HandleMouse1(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        // dont give up mouse once its held until its released
        if (m_mouseDown)
            return true;

        // hack currently
        if (m_focusedEditBox)
            m_focusedEditBox->ClearFocus();

        // todo: handle/add layers
        for (auto& tree : m_domTrees) {
            UIFrame* frame = tree->HitTest(m_mouseX, m_mouseY);
            if (frame) {
                frame->OnClick();
                m_mouseDown = true;
                break;
            }
        }
        return m_mouseDown;
    }
    m_mouseDown = false;
    return false;
}

bool UIManager::HandleMouse2(const input::InputContextCallbackArgs& args) {
    // just need to block mouse if its over ui
    if (args.value > 0) {
        for (auto& tree : m_domTrees) {
            UIFrame* frame = tree->HitTest(m_mouseX, m_mouseY);
            if (frame) {
                m_mouse2Down = true;
                break;
            }
        }
        return m_mouse2Down;
    }
    m_mouse2Down = false;
    return false;
}

void UIManager::UpdateViewport(const Viewport& vp) {
    m_viewport = vp;
    // TODO: update framescales
}

void UIManager::AddFrameObj(uint64_t key, UI* ui, Spatial* spatial) {
    assert(ui);
    assert(spatial);

    std::vector<UIFrame*> frames;
    for (auto& uiFrameUP : ui->frames) {
        UIFrame* uiFrame = uiFrameUP.get();

        if (uiFrame->GetParent()) {
            m_domTrees.back()->InsertFrame(uiFrame);
        }
        else {
            m_domTrees.push_back(std::make_unique<UIDomTree>(m_textRenderer, m_uiRenderer, m_viewport, ui->isWorldFrame));
            m_domTrees.back()->SetRoot(uiFrame);
            m_domTrees.back()->SetPos(spatial->pos, spatial->direction);
        }
        frames.emplace_back(uiFrame);
    }
    m_uiFrames[key] = std::move(frames);

    // initialize frames
    for (auto& frame : m_uiFrames[key])
        frame->InitializeScriptHandler(&m_scriptApi);
}

void UIManager::ProcessFrames() {

    // Dumb way to handle edit box focus', oldest frame gets it first
    // If we ignore a 'wantsfocus' it will assume it has it after we call 'doupdate'
    bool wantsFocus = false;
    for (auto& p : m_uiFrames) {
        for (auto& uiFrame : p.second) {
            if (uiFrame->GetFrameType() == FrameType::EDITBOX) {
                if (!wantsFocus && ((EditBox*)uiFrame)->WantsFocus()) {
                    wantsFocus = true;
                    m_focusedEditBox = (EditBox*)uiFrame;
                    m_drawCaret = true;
                    m_keyboardManager->RestartCapture(m_focusedEditBox->GetText(), m_focusedEditBox->GetCursor());
                    m_cursorBlink = m_focusedEditBox->GetBlinkRate();
                }
            }
        }
    }
    if (wantsFocus) {
        for (auto& p : m_uiFrames) {
            for (auto& uiFrame : p.second) {
                if (uiFrame->GetFrameType() == FrameType::EDITBOX && uiFrame != m_focusedEditBox) {
                    ((EditBox*)uiFrame)->ClearFocus();
                }
            }
        }
    }
}

// This returns first frame with given name
// todo: deal with multiple somehow, or don't do this
UIFrame* UIManager::GetFrame(std::string_view name) {
    for (auto& p : m_uiFrames) {
        for (auto& uiFrame : p.second) {
            if (uiFrame->GetFrameName() == name) {
                return uiFrame;
            }
        }
    }
    LOG_D("UIManager::GetFrame: '%s' not found!", name)
    return nullptr;
}

void UIManager::PreProcess() {
    // Before actual render, lets set text for edit box
    if (m_focusedEditBox) {
        // if enter pressed, trigger editbox
        // we do it here so that text can reset during the process event
        if (m_keyboardManager->HasMessage()) {
            m_focusedEditBox->EnterPressed();
            // text may have changed, so reset capture
            m_keyboardManager->RestartCapture(m_focusedEditBox->GetText(), m_focusedEditBox->GetText().length());
        }

        // when text changes, reset cursor blink
        // idk, every other program does, so why not us?
        if (m_keyboardManager->TextHasChanged()) {
            std::string text = m_keyboardManager->GetText();
            m_focusedEditBox->SetText(text);
            m_cursorBlink = m_focusedEditBox->GetBlinkRate();
            m_drawCaret   = true;
        }
    }
}

void UIManager::PostProcess(float ms) {
    // Double check focusbox
    if (m_focusedEditBox && !m_focusedEditBox->HasFocus()) {
        m_focusedEditBox = nullptr;
        m_cursorBlink    = 0;
        m_drawCaret      = false;
        m_keyboardManager->StopCapture();
    }

    if (m_cursorBlink > 0)
        m_cursorBlink -= ms;

    if (m_focusedEditBox) {
        m_focusedEditBox->SetText(m_keyboardManager->GetText());
        m_focusedEditBox->SetCursor(m_keyboardManager->GetCursorPosition());
        // set caret if necessary
        if (m_cursorBlink <= 0) {
            m_drawCaret = !m_drawCaret;
            m_cursorBlink = m_focusedEditBox->GetBlinkRate();
        }
    }

    for (auto& tree : m_domTrees) {
        tree->SetFocus(m_focusedEditBox);
        tree->RenderTree(m_drawCaret);
        if (m_debugDrawFocus && m_focusedEditBox) {
            dm::Rect2Df rect = tree->GetRenderedSize(m_focusedEditBox);
            if (rect.bl() != rect.tr()) {
                m_debugRenderer->AddRect2D(rect, { 1.f, 0.f, 0.f });
            }
        }
    }
}

void UIManager::ProcessEvents() {

    for (auto& p : m_uiFrames) {
        for (auto& frame : p.second) {
            auto& wantedEvents = frame->GetRegisteredEvents();
            for (auto& ename : wantedEvents) {
                auto check = m_pendingUIEvents.find(ename);
                if (check != m_pendingUIEvents.end()) {
                    for (auto& e : check->second) {
                        frame->OnEvent(check->first, e.args);
                    }
                }
            }
        }
    }

    m_pendingUIEvents.clear();
}

void UIManager::DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) {
    assert(components[ComponentType::UI] != nullptr);
    assert(components[ComponentType::Spatial] != nullptr);
    auto& uis = *reinterpret_cast<const std::array<std::unique_ptr<UI>, MAX_SIM_OBJECTS>*>(components[ComponentType::UI]);
    auto& spatials = *reinterpret_cast<const std::array<std::unique_ptr<Spatial>, MAX_SIM_OBJECTS>*>(components[ComponentType::Spatial]);
    for (size_t i = 0; i < MAX_SIM_OBJECTS; ++i) {
        UI* ui = uis[i].get();
        Spatial* spatial = spatials[i].get();
        if (ui != nullptr && spatial != nullptr) {
            // todo: handle updates from components
            if (m_uiFrames.find(i) == m_uiFrames.end())
                AddFrameObj((uint64_t)i, ui, spatial);
        }
    }

    PreProcess();

    // Process hide/show/focus and render
    ProcessFrames();

    ProcessEvents();

    for (auto& p : m_uiFrames) {
        for(auto& frame : p.second)
            frame->DoUpdate(ms);
    }

    PostProcess(ms);
}
}
