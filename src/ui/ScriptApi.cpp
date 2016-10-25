#include "ScriptApi.h"
#include "UIManager.h"

namespace ui {
    UIFrame* ScriptApi::GetFrame(std::string name) {
        return m_uiManager->GetFrame(name);
    }
}