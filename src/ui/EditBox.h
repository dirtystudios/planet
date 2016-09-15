#pragma once
#include "UIFrame.h"
#include "FontDesc.h"
#include <string>

namespace ui {

class EditBox : public UIFrame {
public:
    struct EditBoxDesc : UIFrameDesc {
        float blinkSpeed; // in ms
        std::string intialText;
        // bool multiLine;
        FontDesc font;
    };
    struct HighLightState {
        uint32_t start, end;
        std::string text;
    };

private:
    enum TextBoxState {
        UNFOCUSED = 0,
        PENDINGCLEAR,
        PENDINGFOCUS,
        FOCUSED,
    };
    std::string m_contents;
    uint32_t m_cursorPos;
    TextBoxState m_textBoxState;
    HighLightState m_highlightState;
    EditBoxDesc m_editBoxDesc;

public:
    EditBox(EditBoxDesc editBoxDesc);
    EditBox(EditBoxDesc editBoxDesc, ScriptHandler* scriptHandler);
    void SetFocus();
    void ClearFocus();
    bool HasFocus();
    float GetBlinkRate();

    void SetColor(float* color);
    float* GetColor();

    std::string GetText() const;
    void SetText(std::string text);
    void AppendText(std::string text);
    void HighlightText(uint32_t start, uint32_t end);
    HighLightState GetHighLightText();
    void ClearText();

    void SetCursor(uint32_t pos);
    uint32_t GetCursor();

    void OnClick();
    void EnterPressed();
    // Called by UIManager
    void DoUpdate(float ms);
    bool WantsFocus();
};
}