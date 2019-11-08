#pragma once
#include <string>

// this is used as an interface for the 'global' script functions that are available to scripts
namespace ui {
    class UIManager;
    class UIFrame;
    class ScriptApi {
    private:
        UIManager* m_uiManager;
    public:
        ScriptApi(UIManager* uiManager)
            : m_uiManager(uiManager) {}
        UIFrame* GetFrame(std::string_view name);
    };
}