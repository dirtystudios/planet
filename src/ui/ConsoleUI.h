#pragma once
#include <memory>
#include "UIFrame.h"
#include "UIManager.h"
#include "EditBox.h"

#include "ConsoleCommands.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
    class ConsoleUI {
    private:
        class ConsoleScriptHandler : public ScriptHandler {
            void OnEnterPressed(EditBox &frame) {
                config::ConsoleCommands* cc = &config::ConsoleCommands::getInstance();
                std::string retMessage = cc->ProcessConsoleString(frame.GetText());
                frame.ClearText();
            }
        };

        std::unique_ptr<ConsoleScriptHandler> m_scriptHandler;
        UIFrame *consoleFrame;
        EditBox *editBox;
    public:
        ConsoleUI(UIManager* uiManager, input::InputContext* inputContext) {
            inputContext->BindContext<input::ContextBindingType::Action>("ToggleConsole", BIND_MEM_CB(&ConsoleUI::HandleConsoleKey, this));
            UIFrame::UIFrameDesc consoleFrameDesc;
            consoleFrameDesc.name = "ConsoleFrame";
            consoleFrameDesc.parent = 0;
            consoleFrameDesc.height = 200;
            consoleFrameDesc.width = 800;
            consoleFrameDesc.x = 0.f;
            consoleFrameDesc.y = 400.f;
            consoleFrameDesc.shown = false;
            consoleFrameDesc.acceptMouse = true;
            consoleFrame = new UIFrame(consoleFrameDesc);
            uiManager->AddFrame(consoleFrame);

            m_scriptHandler.reset(new ConsoleScriptHandler());

            EditBox::EditBoxDesc editBoxDesc;
            editBoxDesc.name = "ConsoleEditBox";
            editBoxDesc.parent = consoleFrame;
            editBoxDesc.height = 40;
            editBoxDesc.width = 780;
            editBoxDesc.x = 10.f;
            editBoxDesc.y = 410;
            editBoxDesc.textSize = 12.f;
            editBoxDesc.blinkSpeed = 0.f;
            editBoxDesc.shown = true;
            editBoxDesc.acceptMouse = true;
            editBox = new EditBox(editBoxDesc, m_scriptHandler.get());
            uiManager->AddFrame(editBox);
        }

        bool HandleConsoleKey(const input::InputContextCallbackArgs& args) {
            if (consoleFrame->IsShown()) {
                consoleFrame->Hide();
                editBox->ClearFocus();
            }
            else {
                consoleFrame->Show();
                editBox->SetFocus();
            }
            return true;
        }
    };
}