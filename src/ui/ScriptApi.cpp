#include "ScriptApi.h"
#include "UIManager.h"

namespace ui {
    UIFrame* ScriptApi::GetFrame(std::string_view name) {
        return m_uiManager->GetFrame(name);
    }
}