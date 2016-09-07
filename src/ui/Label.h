#pragma once
#include "UIFrame.h"
#include "FontDesc.h"
#include <string>

namespace ui {
    class Label : public UIFrame {
    public:
        struct LabelDesc : UIFrameDesc {
            std::string text;
            FontDesc font;
        };

    private:
        LabelDesc m_labelDesc;
        std::string m_contents;
    public:
        Label(LabelDesc labelDesc);
        Label(LabelDesc labelDesc, ScriptHandler* scriptHandler);

        std::string GetText() const;
        float* GetColor();

        // Called by UIManager
        void DoUpdate(float ms) {};
    };
}