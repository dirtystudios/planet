#pragma once
#include "ScriptApi.h"

// Implement this thing and give it to a frame if you want 'callbacks' to happen on events
// Not super thrilled with this way of doing things, but using it for a quick concept

namespace ui {
    class UIFrame;
    class BaseScriptHandler {
    private:
        ScriptApi* m_scriptApi{ nullptr };
    public:
        void Initialize(ScriptApi* scriptApi) {
            m_scriptApi = scriptApi;
        }

        UIFrame* GetFrame(std::string name) {
            return m_scriptApi->GetFrame(name);
        }

        // 'Base Frame API' calls
        virtual void OnLoad(UIFrame& frame) = 0;
    };
}