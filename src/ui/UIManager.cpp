#include "UIManager.h"

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

void UIManager::UpdateViewport(Viewport viewport) {
    m_viewport = viewport;
    // TODO: update framescales
}

void UIManager::AddFrameObj(SimObj* frameObj) {
    UI* ui = frameObj->GetComponent<UI>(ComponentType::UI);
    assert(ui);

    for (auto& uiFrameUP : ui->frames) {
        UIFrame* uiFrame = uiFrameUP.get();

        if (uiFrame->GetParent()) {
            m_domTrees.back().get()->InsertFrame(uiFrame);
        }
        else {
            m_domTrees.push_back(std::make_unique<UIDomTree>(m_textRenderer, m_uiRenderer, m_viewport));
            m_domTrees.back().get()->SetRoot(uiFrame);
        }
        m_uiFrames.emplace_back(uiFrame);
    }
}

void UIManager::ProcessFrames() {

    // Dumb way to handle edit box focus', oldest frame gets it first
    // If we ignore a 'wantsfocus' it will assume it has it after we call 'doupdate'
    bool wantsFocus = false;
    for (auto& uiFrame : m_uiFrames) {
        if (uiFrame->GetFrameType() == FrameType::EDITBOX) {
            if (!wantsFocus && ((EditBox*)uiFrame)->WantsFocus()) {
                wantsFocus       = true;
                m_focusedEditBox = (EditBox*)uiFrame;
                m_drawCaret      = true;
                m_keyboardManager->RestartCapture(m_focusedEditBox->GetText(), m_focusedEditBox->GetCursor());
                m_cursorBlink = m_focusedEditBox->GetBlinkRate();
            }
        }
    }
    if (wantsFocus) {
        for (auto& uiFrame : m_uiFrames) {
            if (uiFrame->GetFrameType() == FrameType::EDITBOX && uiFrame != m_focusedEditBox) {
                ((EditBox*)uiFrame)->ClearFocus();
            }
        }
    }
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
        m_focusedEditBox = 0;
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
        tree->RenderTree({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, m_drawCaret);
    }
}

void UIManager::DoUpdate(float ms) {
    PreProcess();

    // Process hide/show/focus and render
    ProcessFrames();

    for (auto& uiFrame : m_uiFrames) {
        uiFrame->DoUpdate(ms);
    }

    PostProcess(ms);
}
}
