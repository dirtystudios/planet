#pragma once
#include "UIFrame.h"
#include "UIManager.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
    class ConsoleUI {
    private:
        UIFrame *consoleFrame;
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
            consoleFrame = new UIFrame(consoleFrameDesc, true);
            consoleFrame->Hide();
            uiManager->AddFrame(consoleFrame);
        }

        bool HandleConsoleKey(float value) {
            if (consoleFrame->IsShown())
                consoleFrame->Hide();
            else 
                consoleFrame->Show();
            return true;
        }
    };
}