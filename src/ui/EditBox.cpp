#include "EditBox.h"

namespace ui {

    EditBox::EditBox(EditBoxDesc editBoxDesc)
        : UIFrame(editBoxDesc),
          m_textBoxState(TextBoxState::UNFOCUSED),
          m_cursorPos(0),
          m_editBoxDesc(editBoxDesc) {
        m_frameType = FrameType::EDITBOX;
        if (editBoxDesc.blinkSpeed <= 0.0f) {
            m_editBoxDesc.blinkSpeed = 530;
        }
        if (!editBoxDesc.color) {
            m_color[0] = m_color[1] = m_color[2] = 1.f;
        }
    }

    void EditBox::SetFocus() {
        if (!m_editBoxDesc.shown)
            return;

        // this state means we got focus back before acknowledging it
        if (m_textBoxState == TextBoxState::WANTSCLEAR)
            m_textBoxState = TextBoxState::FOCUSED;

        // otherwise we still just 'want' focus
        if (m_textBoxState != TextBoxState::FOCUSED)
            m_textBoxState = TextBoxState::WANTSFOCUS;
    }

    void EditBox::ClearFocus() {
        switch (m_textBoxState) {
        case TextBoxState::FOCUSED:
            m_textBoxState = TextBoxState::WANTSCLEAR;
            break;
        case TextBoxState::WANTSFOCUS:
            m_textBoxState = TextBoxState::UNFOCUSED;
            break;
        default:
            break;
        }
    }

    std::string EditBox::GetText() {
        return m_contents;
    }

    float EditBox::GetBlinkRate() {
        return m_editBoxDesc.blinkSpeed;
    }

    void EditBox::SetText(std::string text) {
        m_contents = text;
    }

    void EditBox::SetColor(float *color) {
        m_color[0] = color[0];
        m_color[1] = color[1];
        m_color[2] = color[2];
    }

    float* EditBox::GetColor() {
        return m_color;
    }

    void EditBox::AppendText(std::string text) {
        m_contents += text;
    }

    void EditBox::HighlightText(uint32_t start, uint32_t end) {
        m_highlightState.start = start;
        m_highlightState.end = end;
        m_highlightState.text = m_contents.substr(start, end - start);
    }

    EditBox::HighLightState EditBox::GetHighLightText() {
        return m_highlightState;
    }

    void EditBox::ClearText() {
        m_contents = "";
        m_contents.clear();
    }

    void EditBox::SetCursor(uint32_t pos) {
        m_cursorPos = pos;
    }

    uint32_t EditBox::GetCursor() {
        return m_cursorPos;
    }

    bool EditBox::HasFocus() {
        return (m_textBoxState == TextBoxState::FOCUSED);
    }

	void EditBox::OnClick() {
		SetFocus();
	}

    // Called by UIManager
    void EditBox::DoUpdate(float ms) {
        if (!m_editBoxDesc.shown)
            m_textBoxState = TextBoxState::UNFOCUSED;

        if (m_textBoxState == TextBoxState::WANTSFOCUS)
            m_textBoxState = TextBoxState::FOCUSED;
        if (m_textBoxState == TextBoxState::WANTSCLEAR)
            m_textBoxState = TextBoxState::UNFOCUSED;
    }

    bool EditBox::WantsFocus() {
        return (m_textBoxState == TextBoxState::WANTSFOCUS);
    }
}