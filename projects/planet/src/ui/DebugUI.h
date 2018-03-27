#pragma once

#include "UI.h"
#include "KeyValueList.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
    class DebugUI {

        KeyValueList* _kvList;
    public:
        DebugUI(UI* uiFrameObj) {
            UIFrame::UIFrameDesc frameDesc;
            frameDesc.acceptMouse = false;
            frameDesc.width = 100;
            frameDesc.height = 80;
            frameDesc.x = 650.f;
            frameDesc.y = 200.f;
            frameDesc.show = true;
            KeyValueList kvList(frameDesc);

            uiFrameObj->frames.push_back(std::make_unique<KeyValueList>(kvList));
            _kvList = (KeyValueList*)uiFrameObj->frames.back().get();
        }

        void AddKeyValue(std::string key, std::string value) {
            _kvList->InsertKey(key, value);
        }
    };
}
