#pragma once
#include <memory>
#include "ConsoleCommands.h"
#include "EditBox.h"
#include "InputContext.h"
#include "UI.h"
#include "UIFrame.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
class ConsoleUI {
private:
    class ConsoleScriptHandler : public ScriptHandler {
        void OnEnterPressed(EditBox& frame) {
            config::ConsoleCommands* cc         = &config::ConsoleCommands::getInstance();
            std::string              retMessage = cc->ProcessConsoleString(frame.GetText());
            frame.ClearText();
        }
    };

    std::unique_ptr<ConsoleScriptHandler> m_scriptHandler;
    UIFrame*                              consoleFrame;
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
        uiFrameObj->frames.push_back(std::make_unique<UIFrame>(consoleFrameDesc));

        m_scriptHandler.reset(new ConsoleScriptHandler());

        consoleFrame = uiFrameObj->frames.back().get();

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
        uiFrameObj->frames.push_back(std::make_unique<EditBox>(editBoxDesc, m_scriptHandler.get()));

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
