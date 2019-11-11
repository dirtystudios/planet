#include "ScriptApi.h"
#include "UIManager.h"

namespace ui {
    UIFrame* ScriptApi::GetFrame(std::string_view name) {
        return m_uiManager->GetFrame(name);
    }

    void ScriptApi::SendChatMessage(std::string_view msg) {
        m_uiManager->HandleUIEvent(UIEvent{ "CHAT_MSG_SEND", {std::string(msg)} });
    }
}