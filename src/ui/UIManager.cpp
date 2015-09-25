#include "UIManager.h"

namespace ui {
    void UIManager::AddFrame(UIFrame* uiFrame) {
        m_frameTree.emplace(uiFrame->GetParent(), uiFrame);
        m_uiFrames.emplace_back(uiFrame);
    }

    void UIManager::RenderFrame(UIFrame* uiFrame) {
        UIFrame::UIFrameDesc* frameDesc = uiFrame->GetFrameDesc();
        m_uiRenderer->RenderFrame(frameDesc->x, frameDesc->y,
            frameDesc->width, frameDesc->height);
        if (uiFrame->GetFrameType() == FrameType::EDITBOX) {
            m_textRenderer->RenderText(((EditBox*)uiFrame)->GetText(), frameDesc->x, frameDesc->y, 1.0, glm::vec3(0.9, 0.9, 0.9));
        }
    }

    void UIManager::ProcessFrames() {
        // todo: this probly can be optimized better eventualy

        // K weed out frames not set to shown and any children
        auto m_tempTree = m_frameTree;
        auto m_tempList = m_uiFrames;
        for (auto it = m_frameTree.begin(), end = m_frameTree.end(); it != end; it = m_frameTree.upper_bound(it->first)) {
            // this *should* loop through just unique keys
            // Remove any keys from tempTree that arent shown, and any children, and their children....etc
            if (it->first == 0)
                continue;

            if (!it->first->IsShown()) {
                RemoveChildren(it->first, m_tempTree);
                m_tempTree.erase(it->first);
            }
        }

        // Dumb way to handle edit box focus', oldest frame gets it first
        // If we ignore a 'wantsfocus' it will assume it has it after we call 'doupdate'
        bool wantsFocus = false;
        for (auto it = m_tempTree.begin(); it != m_tempTree.end(); ++it) {
            if (it->first == 0)
                continue;
            if (it->second->GetFrameType() == FrameType::EDITBOX) {
                if (!wantsFocus && ((EditBox*)it->second)->WantsFocus()) {
                    wantsFocus = true;
                    m_focusedEditBox = (EditBox*)it->second;
                    m_drawCaret = true;
                    m_keyboardManager->RestartCapture(m_focusedEditBox->GetText());
                    m_cursorBlink = m_focusedEditBox->GetBlinkRate();
                }
            }
        }
        if (wantsFocus) {
            for (auto it = m_tempTree.begin(); it != m_tempTree.end(); ++it) {
                if (it->first == 0)
                    continue;
                if (it->second->GetFrameType() == FrameType::EDITBOX 
                    && it->second != m_focusedEditBox) {
                    ((EditBox*)it->second)->ClearFocus();
                }
            }
        }


        // K so, only 'shown' frames should be left
        for (auto it = m_tempTree.begin(), end = m_tempTree.end(); it != end; it = m_tempTree.upper_bound(it->first)) {
            if (it->first == 0)
                continue;
            RenderChildren(it->first, m_tempTree);
        }
    }

    void UIManager::DoUpdate(float dt) {
        ProcessFrames();
        for (UIFrame* frame : m_uiFrames) {
            frame->DoUpdate(dt);
        }

        // Double check focusbox
        if (m_focusedEditBox && !m_focusedEditBox->HasFocus()) {
            m_focusedEditBox = 0;
            m_cursorBlink = 0;
            m_drawCaret = false;
            m_keyboardManager->StopCapture();
        }

        if (m_cursorBlink > 0)
            m_cursorBlink -= dt;

        // Draw Caret
        if (m_focusedEditBox) {
            if (m_cursorBlink <= 0) m_drawCaret = !m_drawCaret;
            m_cursorBlink = m_focusedEditBox->GetBlinkRate();
            std::string text = m_keyboardManager->GetText();
            m_focusedEditBox->SetText(text);
            if (m_drawCaret) {
                //m_textRenderer->DrawCaret(0.0, 0.0, 1.f);
            }
        }
    }
}