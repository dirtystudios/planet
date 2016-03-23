#include "UIManager.h"

namespace ui {

	bool UIManager::HandleMouseX(float xPos) {
		m_mouseX = xPos;
		return m_mouseDown;
	}

	bool UIManager::HandleMouseY(float yPos) {
		m_mouseY = yPos;
		return m_mouseDown;
	}

	bool UIManager::HandleMouse1(float value) {
		if (value > 0) {
			// ok.... we 'clicked', check if its within a frame and something we care about
			// todo: handle/add layers and parent child relations
			for (auto &uiFrame : m_uiFrames) {
				if (uiFrame->AcceptingInput()) {
					UIFrame::UIFrameDesc* frameDesc = uiFrame->GetFrameDesc();
					if (m_mouseX > frameDesc->x
						&& m_mouseX < (frameDesc->x + frameDesc->width)
						&& m_mouseY > frameDesc->y
						&& m_mouseY < (frameDesc->y + frameDesc->height)) {

						uiFrame->OnClick();
						m_mouseDown = true;
						return true;
					}
				}
			}
		}
		m_mouseDown = false;
		return false;
	}

    void UIManager::AddFrame(UIFrame* uiFrame) {
        m_frameTree.emplace(uiFrame->GetParent(), uiFrame);
        m_uiFrames.emplace_back(uiFrame);
    }

    void UIManager::RenderFrame(UIFrame* uiFrame) {
        UIFrame::UIFrameDesc* frameDesc = uiFrame->GetFrameDesc();
        m_uiRenderer->RenderFrame(frameDesc->x, frameDesc->y,
            frameDesc->width, frameDesc->height);
        if (uiFrame->GetFrameType() == FrameType::EDITBOX) {
            float *tempColor = ((EditBox*)uiFrame)->GetColor();
            glm::vec3 color = { tempColor[0], tempColor[1], tempColor[2] };
            m_textRenderer->RenderText(((EditBox*)uiFrame)->GetText(), frameDesc->x, frameDesc->y, 1.0, color);
        }
    }

    void UIManager::ProcessFrames() {
        // todo: this probly can be optimized better eventualy
        // yea, this whole thing should be burned in a fiery blaze

        // K weed out frames not set to shown and any children
        auto m_tempTree = m_frameTree;
        auto m_tempList = m_uiFrames;
        for (auto it = m_frameTree.begin(), end = m_frameTree.end(); it != end; it = m_frameTree.upper_bound(it->first)) {
            // this *should* loop through just unique keys
            // Remove any keys from tempTree that arent shown, and any children, and their children....etc
            if (it->first == 0)
                continue;

            // Handle parent and any children
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
                    // todo: where should cursor start? should save state?
                    m_keyboardManager->RestartCapture(m_focusedEditBox->GetText(), 0);
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

        // K so, only 'shown' parents should be left, renderchildren handles the leaf's
        for (auto it = m_tempTree.begin(), end = m_tempTree.end(); it != end; it = m_tempTree.upper_bound(it->first)) {
            if (it->first == 0)
                continue;
            RenderChildren(it->first, m_tempTree);
        }
    }

    void UIManager::PreProcess() {
        // Before actual render, lets set text for edit box
        if (m_focusedEditBox) {
            // when text changes, reset cursor blink
            // idk, every other program does, so why not us?
            if (m_keyboardManager->TextHasChanged()) {
                std::string text = m_keyboardManager->GetText();
                m_focusedEditBox->SetText(text);
                m_cursorBlink = m_focusedEditBox->GetBlinkRate();
                m_drawCaret = true;
            }
        }
    }

    void UIManager::PostProcess(float ms) {
        // Double check focusbox
        if (m_focusedEditBox && !m_focusedEditBox->HasFocus()) {
            m_focusedEditBox = 0;
            m_cursorBlink = 0;
            m_drawCaret = false;
            m_keyboardManager->StopCapture();
        }

        if (m_cursorBlink > 0)
            m_cursorBlink -= ms;

        // Draw Caret if necessary
        if (m_focusedEditBox) {
            if (m_cursorBlink <= 0) {
                m_drawCaret = !m_drawCaret;
                m_cursorBlink = m_focusedEditBox->GetBlinkRate();
            }
            if (m_drawCaret) {
                UIFrame::UIFrameDesc *temp = m_focusedEditBox->GetFrameDesc();
                std::string text = m_keyboardManager->GetText();
                // TODO: use scale and color?
                m_textRenderer->RenderCursor(text,
                    m_keyboardManager->GetCursorPosition(),
                    temp->x, temp->y, 1.f, glm::vec3(1.f, 1.f, 1.f));
            }
        }
    }

    void UIManager::DoUpdate(float ms) {
        PreProcess();

        // Process hide/show/focus and render
        ProcessFrames();
        for (UIFrame* frame : m_uiFrames) {
            frame->DoUpdate(ms);
        }

        PostProcess(ms);
    }
}