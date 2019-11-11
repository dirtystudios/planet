#pragma once
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
    class ChatFrameUI {
    private:
        class ChatFrameEditBoxScriptHandler : public EditBoxScriptHandler {
        private:
        public:
            void OnLoad(UIFrame& frame) {
            }

            void OnEnterPressed(EditBox& frame) {
                SendChatMessage(frame.GetText());
                frame.ClearText();
            }
        };

        class ChatFrameScriptHandler : public BaseScriptHandler {
        private:
            TextList* consoleFrame{ nullptr };

        public:
            void OnLoad(UIFrame& frame) {
                frame.RegisterEvent("MSG_CHAT");
                consoleFrame = static_cast<TextList*>(GetFrame("ChatFrameTextList"));
            }

            void OnEvent(UIFrame& frame, std::string_view eventName, const std::vector<std::string>& eventData) {
                if (eventName == "MSG_CHAT") {
                    if (eventData.size() > 0)
                        consoleFrame->InsertTextLine(eventData[0]);
                }
            }
        };

        ChatFrameEditBoxScriptHandler           editBoxSC;
        ChatFrameScriptHandler             frameSC;
        UIFrame* chatFrame{ nullptr };
        TextList* textList{ nullptr };
        EditBox* editBox{ nullptr };

    public:
        ChatFrameUI(UI* uiFrameObj, input::InputContext* inputContext) {
            inputContext->BindContext<input::ContextBindingType::Action>("ToggleChatEdit", std::bind(&ChatFrameUI::HandleEnterKey, this, std::placeholders::_1));

            UIFrame::UIFrameDesc chatframeDesc;
            chatframeDesc.name = "Chatframe";
            chatframeDesc.parent = 0;
            chatframeDesc.height = 100;
            chatframeDesc.width = 200;
            chatframeDesc.x = 700.f;
            chatframeDesc.y = 0.f;
            chatframeDesc.show = true;
            chatframeDesc.acceptMouse = true;
            uiFrameObj->frames.emplace_back(std::make_unique<UIFrame>(chatframeDesc, &frameSC));

            chatFrame = uiFrameObj->frames.back().get();

            TextList::TextListDesc textListDesc;
            textListDesc.name = "ChatFrameTextList";
            textListDesc.parent = chatFrame;
            textListDesc.height = 100;
            textListDesc.width = 200;
            textListDesc.x = 0.f;
            textListDesc.y = 0.f;
            textListDesc.show = true;
            uiFrameObj->frames.emplace_back(std::make_unique<TextList>(textListDesc));

            EditBox::EditBoxDesc editBoxDesc;
            editBoxDesc.name = "ChatFrameEditBox";
            editBoxDesc.parent = chatFrame;
            editBoxDesc.height = 40;
            editBoxDesc.width = 780;
            editBoxDesc.x = 0.f;
            editBoxDesc.y = -10.f;
            editBoxDesc.font.textSize = 12.f;
            editBoxDesc.blinkSpeed = 0.f;
            editBoxDesc.show = false;
            editBoxDesc.acceptMouse = true;
            uiFrameObj->frames.emplace_back(std::make_unique<EditBox>(editBoxDesc, &editBoxSC));

            editBox = static_cast<EditBox*>(uiFrameObj->frames.back().get());
        }

        bool HandleEnterKey(const input::InputContextCallbackArgs& args) {
            if (editBox->IsShown()) {
                //editBox->ClearFocus();
            }
            else {
                editBox->Show();
                editBox->SetFocus();
            }
            return true;
        }
    };
}
