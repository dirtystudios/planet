#pragma once
#include <memory>
#include "UIFrame.h"
#include "EditBox.h"
#include "UI.h"

#include "ConsoleCommands.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
class ConsoleUI {
private:
    class ConsoleScriptHandler : public ScriptHandler {
        void OnEnterPressed(EditBox& frame) {
            config::ConsoleCommands* cc = &config::ConsoleCommands::getInstance();
            std::string retMessage = cc->ProcessConsoleString(frame.GetText());
            frame.ClearText();
        }
    };

    std::unique_ptr<ConsoleScriptHandler> m_scriptHandler;
    UIFrame* consoleFrame;
    EditBox* editBox;

public:
    ConsoleUI(UI* uiFrameObj, input::InputContext* inputContext) {
        inputContext->BindContext<input::ContextBindingType::Action>("ToggleConsole",
                                                                     BIND_MEM_CB(&ConsoleUI::HandleConsoleKey, this));
        
        // This keeps references valid cause of stupidity below
        uiFrameObj->frames.reserve(2);

        UIFrame::UIFrameDesc consoleFrameDesc;
        consoleFrameDesc.name        = "ConsoleFrame";
        consoleFrameDesc.parent      = 0;
        consoleFrameDesc.height      = 200;
        consoleFrameDesc.width       = 800;
        consoleFrameDesc.x           = 0.f;
        consoleFrameDesc.y           = 400.f;
        consoleFrameDesc.shown       = false;
        consoleFrameDesc.acceptMouse = true;
        uiFrameObj->frames.push_back(std::make_unique<UIFrame>(consoleFrameDesc));

        m_scriptHandler.reset(new ConsoleScriptHandler());

        consoleFrame = uiFrameObj->frames[0].get();

        EditBox::EditBoxDesc editBoxDesc;
        editBoxDesc.name        = "ConsoleEditBox";
        editBoxDesc.parent      = consoleFrame;
        editBoxDesc.height      = 40;
        editBoxDesc.width       = 780;
        editBoxDesc.x           = 10.f;
        editBoxDesc.y           = 410;
        editBoxDesc.textSize    = 12.f;
        editBoxDesc.blinkSpeed  = 0.f;
        editBoxDesc.shown       = true;
        editBoxDesc.acceptMouse = true;
        uiFrameObj->frames.push_back(std::make_unique<EditBox>(editBoxDesc, m_scriptHandler.get()));

        editBox = static_cast<EditBox*>(uiFrameObj->frames[1].get());
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
