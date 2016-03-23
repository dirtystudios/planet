#pragma once
#include "UIFrame.h"
#include <vector>

namespace ui {
    
    class EditBox : public UIFrame {
    public:
        struct EditBoxDesc : UIFrameDesc {
            float textSize; // ignored
            float blinkSpeed; // in ms
            float* color=0; // rgb, null = default
            //bool multiLine;
        };
        struct HighLightState {
            uint32_t start, end;
            std::string text;
        };
    private: 
        enum TextBoxState {
            UNFOCUSED = 0,
            WANTSCLEAR,
            WANTSFOCUS,
            FOCUSED,
        };
        std::string m_contents;
        uint32_t m_cursorPos;
        TextBoxState m_textBoxState;
        HighLightState m_highlightState;
        EditBoxDesc m_editBoxDesc;
        float m_color[3];
    public:
        EditBox(EditBoxDesc editBoxDesc, bool enableInput);
        void SetFocus();
        void ClearFocus();
        bool HasFocus();
        float GetBlinkRate();

        void SetColor(float *color);
        float* GetColor();

        std::string GetText();
        void SetText(std::string text);
        void AppendText(std::string text);
        void HighlightText(uint32_t start, uint32_t end);
        HighLightState GetHighLightText();
        void ClearText();

        void SetCursor(uint32_t pos);
        uint32_t GetCursor();

		void OnClick();
        // Called by UIManager
        void DoUpdate(float ms);
        bool WantsFocus();

    };
}