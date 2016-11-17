#pragma once
#include "UIFrame.h"
#include "Label.h"
#include "UI.h"

#include <string>

// In a perfect world something like this would be described in lua/xml

namespace ui {
    class LabelUI {
    private:
        LabelUI();

    public:
        static void AttachLabel(UI* uiFrameObj, std::string text) {
            UIFrame::UIFrameDesc labelFrameDesc;
            labelFrameDesc.name = "LabelFrame";
            labelFrameDesc.parent = 0;
            labelFrameDesc.height = 40;
            labelFrameDesc.width = 100;
            labelFrameDesc.x = 5.f;
            labelFrameDesc.y = 5.f;
            labelFrameDesc.show = true;
            labelFrameDesc.acceptMouse = false;
            uiFrameObj->frames.push_back(std::make_unique<UIFrame>(labelFrameDesc));

            Label::LabelDesc labelDesc;
            labelDesc.name = "LabelDesc";
            labelDesc.parent = uiFrameObj->frames.back().get();
            // height and width is ignored for text for now
            labelDesc.height = 0;
            labelDesc.width = 0;
            labelDesc.x = 1.f;
            labelDesc.y = 1.f;
            labelDesc.font.textSize = 12.f;
            labelDesc.show = true;
            labelDesc.acceptMouse = false;
            labelDesc.text = text;
            uiFrameObj->frames.push_back(std::make_unique<Label>(labelDesc));
        }
    };
}
