#pragma once
#include <memory>
#include "ConsoleCommands.h"
#include "EditBox.h"
#include "InputContext.h"
#include "UI.h"
#include "UIFrame.h"
#include "TextList.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
class ConsoleUI {
private:
    class ConsoleEditBoxScriptHandler : public EditBoxScriptHandler {
    private:
        TextList* consoleFrame{ nullptr };
        
    public:
        void OnLoad(UIFrame& frame) {
            consoleFrame = static_cast<TextList*>(GetFrame("ConsoleTextList"));
        }

        void OnEnterPressed(EditBox& frame) {
            if (!consoleFrame || frame.GetText() == "")
                return;
            config::ConsoleCommands* cc         = &config::ConsoleCommands::getInstance();
            std::string              retMessage = cc->ProcessConsoleString(frame.GetText());

            consoleFrame->InsertTextLine(retMessage);
            frame.ClearText();
        }
    };

    class ConsoleFrameScriptHandler : public BaseScriptHandler {
    private:
        TextList* consoleFrame{ nullptr };

    public:
        void OnLoad(UIFrame& frame) {
            frame.RegisterEvent("MSG_CONSOLE");
            consoleFrame = static_cast<TextList*>(GetFrame("ConsoleTextList"));
        }

        void OnEvent(UIFrame& frame, std::string_view eventName, const std::vector<std::string>& eventData) {
            if (eventName == "MSG_CONSOLE") {
                if (eventData.size() > 0)
                    consoleFrame->InsertTextLine(eventData[0]);
            }
        }
    };

    ConsoleEditBoxScriptHandler           editBoxSC;
    ConsoleFrameScriptHandler             frameSC;
    UIFrame*                              consoleFrame;
    TextList*							  textList;
    EditBox*                              editBox;

public:
    ConsoleUI(UI* uiFrameObj, input::InputContext* inputContext) {
        inputContext->BindContext<input::ContextBindingType::Action>("ToggleConsole", std::bind(&ConsoleUI::HandleConsoleKey, this, std::placeholders::_1));

        UIFrame::UIFrameDesc consoleFrameDesc;
        consoleFrameDesc.name        = "ConsoleFrame";
        consoleFrameDesc.parent      = 0;
        consoleFrameDesc.height      = 200;
        consoleFrameDesc.width       = 800;
        consoleFrameDesc.x           = 0.f;
        consoleFrameDesc.y           = 400.f;
        consoleFrameDesc.show        = false;
        consoleFrameDesc.acceptMouse = true;
        uiFrameObj->frames.emplace_back(std::make_unique<UIFrame>(consoleFrameDesc, &frameSC));

        consoleFrame = uiFrameObj->frames.back().get();

        TextList::TextListDesc textListDesc;
        textListDesc.name = "ConsoleTextList";
        textListDesc.parent = consoleFrame;
        textListDesc.height = 130;
        textListDesc.width = 780;
        textListDesc.x = 10.f;
        textListDesc.y = 60.f;
        textListDesc.show = true;
        uiFrameObj->frames.emplace_back(std::make_unique<TextList>(textListDesc));

        EditBox::EditBoxDesc editBoxDesc;
        editBoxDesc.name          = "ConsoleEditBox";
        editBoxDesc.parent        = consoleFrame;
        editBoxDesc.height        = 40;
        editBoxDesc.width         = 780;
        editBoxDesc.x             = 10.f;
        editBoxDesc.y             = 10.f;
        editBoxDesc.font.textSize = 12.f;
        editBoxDesc.blinkSpeed    = 0.f;
        editBoxDesc.show          = true;
        editBoxDesc.acceptMouse   = true;
        uiFrameObj->frames.emplace_back(std::make_unique<EditBox>(editBoxDesc, &editBoxSC));

        editBox = static_cast<EditBox*>(uiFrameObj->frames.back().get());
    }

    bool HandleConsoleKey(const input::InputContextCallbackArgs& args) {
        if (consoleFrame->IsShown()) {
            consoleFrame->Hide();
            editBox->ClearFocus();
        } else {
            consoleFrame->Show();
            editBox->SetFocus();
        }
        return true;
    }
};
}
