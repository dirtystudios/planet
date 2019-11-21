#include "EditBox.h"

namespace ui {

EditBox::EditBox(EditBoxDesc editBoxDesc)
    : UIFrame(editBoxDesc), m_cursorPos(0), m_textBoxState(TextBoxState::UNFOCUSED), m_editBoxDesc(editBoxDesc) {

    m_frameType = FrameType::EDITBOX;
    m_contents = editBoxDesc.intialText;
    if (editBoxDesc.blinkSpeed <= 0.0f) {
        m_editBoxDesc.blinkSpeed = 530;
    }
}

EditBox::EditBox(EditBoxDesc editBoxDesc, EditBoxScriptHandler* scriptHandler)
    : UIFrame(editBoxDesc, scriptHandler), m_cursorPos(0), m_textBoxState(TextBoxState::UNFOCUSED), 
      m_editBoxDesc(editBoxDesc) {

    m_editScriptHandler = scriptHandler;
    m_frameType = FrameType::EDITBOX;
    m_contents = editBoxDesc.intialText;
    if (editBoxDesc.blinkSpeed <= 0.0f) {
        m_editBoxDesc.blinkSpeed = 530;
    }
}

void EditBox::SetFocus() {
    if (!IsShown())
        return;

    // this state means we got focus back before acknowledging it
    if (m_textBoxState == TextBoxState::PENDINGCLEAR)
        m_textBoxState = TextBoxState::FOCUSED;

    // otherwise we still just 'want' focus
    if (m_textBoxState != TextBoxState::FOCUSED)
        m_textBoxState = TextBoxState::PENDINGFOCUS;
}

void EditBox::ClearFocus() {
    switch (m_textBoxState) {
    case TextBoxState::FOCUSED:
        m_textBoxState = TextBoxState::PENDINGCLEAR;
        break;
    case TextBoxState::PENDINGFOCUS:
        m_textBoxState = TextBoxState::UNFOCUSED;
        break;
    default:
        break;
    }
}

FontDesc EditBox::GetFontDesc() const { return m_editBoxDesc.font; };

std::string EditBox::GetText() const { return m_contents; }

float EditBox::GetBlinkRate() { return m_editBoxDesc.blinkSpeed; }

void EditBox::SetText(std::string text) { m_contents = text; }

void EditBox::SetColor(float* color) {
    m_editBoxDesc.font.color[0] = color[0];
    m_editBoxDesc.font.color[1] = color[1];
    m_editBoxDesc.font.color[2] = color[2];
}

glm::vec3 EditBox::GetColor() const { return m_editBoxDesc.font.color; }

void EditBox::AppendText(std::string text) { m_contents += text; }

void EditBox::HighlightText(uint32_t start, uint32_t end) {
    m_highlightState.start = start;
    m_highlightState.end   = end;
    m_highlightState.text  = m_contents.substr(start, end - start);
}

EditBox::HighLightState EditBox::GetHighLightText() { return m_highlightState; }

void EditBox::ClearText() {
    m_contents = "";
    m_contents.clear();
}

void EditBox::SetCursor(uint32_t pos) { 
    if (pos > m_contents.length())
        m_cursorPos = m_contents.length();
    else 
        m_cursorPos = pos; 
}

uint32_t EditBox::GetCursor() {
    if (m_cursorPos > m_contents.length())
        m_cursorPos = m_contents.length();
    return m_cursorPos; 
}

bool EditBox::HasFocus() { return (m_textBoxState == TextBoxState::FOCUSED); }

void EditBox::OnClick() { SetFocus(); }

// Called by UIManager

void EditBox::EnterPressed() {
    if (m_editScriptHandler) {
        m_editScriptHandler->OnEnterPressed(*this);
    }
}

void EditBox::DoUpdate(float ms) {
    if (!IsShown())
        m_textBoxState = TextBoxState::UNFOCUSED;

    if (m_textBoxState == TextBoxState::PENDINGFOCUS)
        m_textBoxState = TextBoxState::FOCUSED;
    if (m_textBoxState == TextBoxState::PENDINGCLEAR)
        m_textBoxState = TextBoxState::UNFOCUSED;
}

bool EditBox::WantsFocus() { return (m_textBoxState == TextBoxState::PENDINGFOCUS); }
}
