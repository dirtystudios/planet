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
        Label(LabelDesc labelDesc, BaseScriptHandler* scriptHandler);

        FontDesc GetFontDesc() const;

        std::string GetText() const;
        glm::vec3 GetColor() const;

        // Called by UIManager
        void DoUpdate(float ms) {};
    };
}