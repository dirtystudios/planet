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

        // ok.... we 'clicked', check if its within a frame and something we care about
        // todo: handle/add layers and parent child relations
        for (auto& uiFrame : m_uiFrames) {
            if (!uiFrame.first->IsShown())
                continue;
            if (uiFrame.first->GetFrameDesc()->acceptMouse) {
                // k this is a potential candidate, lets figure out its 'scaled' pos
                UIFrame::UIFrameDesc* frameDesc = uiFrame.first->GetFrameDesc();
                FrameScale scaledFrame = uiFrame.first->GetScaledSize(m_viewport);
                if (m_mouseX > scaledFrame.x && m_mouseX < (scaledFrame.x + scaledFrame.width) &&
                    m_mouseY > scaledFrame.y && m_mouseY < (scaledFrame.y + scaledFrame.height)) {

                    uiFrame.first->OnClick();
                    m_mouseDown = true;
                }
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

    for (auto& uiFrame : ui->frames) {
        m_frameTree.emplace(uiFrame->GetParent(), uiFrame.get());

        FrameScale scaled           = uiFrame->GetScaledSize(m_viewport);

        if (uiFrame->GetFrameType() == FrameType::LABEL) {
            // for now label type is always shown
            Label* label = dynamic_cast<Label*>((uiFrame.get()));
            glm::vec3 color = { label->GetColor()[0], label->GetColor()[1], label->GetColor()[2] };
            auto rawr = m_textRenderer->RegisterText(label->GetText(), scaled.x, scaled.y, color);
            m_textFrames.emplace(uiFrame.get(), rawr);
        }
        else if (uiFrame->GetFrameType() == FrameType::EDITBOX) {
            EditBox* ebox = dynamic_cast<EditBox*>((uiFrame.get()));
            glm::vec3 color = { ebox->GetColor()[0], ebox->GetColor()[1], ebox->GetColor()[2] };
            auto rawr = m_textRenderer->RegisterText(ebox->GetText(), scaled.x, scaled.y, color);
            m_textFrames.emplace(uiFrame.get(), rawr);

            UIFrameRenderObj* renderObj = m_uiRenderer->RegisterFrame(uiFrame.get(), scaled);
            m_uiFrames.emplace(uiFrame.get(), renderObj);
            m_parentFrames.emplace(uiFrame->GetParent());
        }
        else {
            UIFrameRenderObj* renderObj = m_uiRenderer->RegisterFrame(uiFrame.get(), scaled);
            m_uiFrames.emplace(uiFrame.get(), renderObj);
            m_parentFrames.emplace(uiFrame->GetParent());
        }
    }
}

void UIManager::ProcessFrames() {
    // todo: this probly can be optimized better eventualy
    // yea, this whole thing should be burned in a fiery blaze

    // K weed out frames not set to shown and any children
    auto m_shownFramesTree = m_frameTree;

    for (auto parentFrame : m_parentFrames) {
        // this *should* loop through just unique keys
        // Remove any keys from tempTree that arent shown, and any children, and their children....etc
        if (parentFrame == 0)
            continue;

        // Handle parent and any children
        if (!parentFrame->GetFrameDesc()->shown) {
            RemoveChildren(parentFrame, m_shownFramesTree);
            m_shownFramesTree.erase(parentFrame);
        }
    }

    // Dumb way to handle edit box focus', oldest frame gets it first
    // If we ignore a 'wantsfocus' it will assume it has it after we call 'doupdate'
    bool wantsFocus = false;
    for (auto it = m_shownFramesTree.begin(); it != m_shownFramesTree.end(); ++it) {
        if (it->first == 0)
            continue;
        if (it->second->GetFrameType() == FrameType::EDITBOX) {
            if (!wantsFocus && ((EditBox*)it->second)->WantsFocus()) {
                wantsFocus       = true;
                m_focusedEditBox = (EditBox*)it->second;
                m_drawCaret      = true;
                // todo: where should cursor start? should save state?
                m_keyboardManager->RestartCapture(m_focusedEditBox->GetText(), 0);
                m_cursorBlink = m_focusedEditBox->GetBlinkRate();
            }
        }
    }
    if (wantsFocus) {
        for (auto it = m_shownFramesTree.begin(); it != m_shownFramesTree.end(); ++it) {
            if (it->first == 0)
                continue;
            if (it->second->GetFrameType() == FrameType::EDITBOX && it->second != m_focusedEditBox) {
                ((EditBox*)it->second)->ClearFocus();
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
        m_drawCaret = false;
        m_keyboardManager->StopCapture();
    }

    if (m_cursorBlink > 0)
        m_cursorBlink -= ms;

    // Draw Caret if necessary
    if (m_focusedEditBox) {
        if (m_cursorBlink <= 0) {
            m_drawCaret   = !m_drawCaret;
            m_cursorBlink = m_focusedEditBox->GetBlinkRate();
        }
        if (m_drawCaret) {
            UIFrame::UIFrameDesc* temp = m_focusedEditBox->GetFrameDesc();
            std::string text           = m_keyboardManager->GetText();

            // eugene: leaving this here until we fix text rendering

            // TODO: color?
            //                FrameScale scaledFrame = GetScaledFrame(temp);
            //                m_textRenderer->RenderCursor(text,
            //                    m_keyboardManager->GetCursorPosition(),
            //                    scaledFrame.x, scaledFrame.y, 1.f, glm::vec3(1.f, 1.f, 1.f));
        }
    }
}

void UIManager::DoUpdate(float ms) {
    PreProcess();

    // Process hide/show/focus and render
    ProcessFrames();
    for (auto& frame : m_uiFrames) {
        frame.first->DoUpdate(ms);
    }

    for (auto& eBox : m_textFrames) {
        if (eBox.first->IsShown()) {
            if (eBox.first->GetFrameType() == FrameType::LABEL)
                eBox.second->SetText(((Label*)eBox.first)->GetText());
            else 
                eBox.second->SetText(((EditBox*)eBox.first)->GetText());
        }
        else eBox.second->SetText("");
    }

    PostProcess(ms);
}
}
