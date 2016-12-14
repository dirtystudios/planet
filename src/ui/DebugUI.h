#pragma once

#include "UI.h"
#include "KeyValueList.h"
#include "UITextureDebug.h"

// In a perfect world something like this would be described in lua/xml

namespace ui {
    class DebugUI {

        KeyValueList* _kvList;
        TextureDebug* _tex;
    public:
        DebugUI(UI* uiFrameObj) {
            UIFrame::UIFrameDesc frameDesc;
            frameDesc.name = "KVDebug";
            frameDesc.acceptMouse = false;
            frameDesc.width = 100;
            frameDesc.height = 80;
            frameDesc.x = 650.f;
            frameDesc.y = 200.f;
            frameDesc.show = true;
            KeyValueList kvList(frameDesc);

            uiFrameObj->frames.push_back(std::make_unique<KeyValueList>(kvList));
            _kvList = (KeyValueList*)uiFrameObj->frames.back().get();

            Texture::TextureDesc texDesc;
            texDesc.name = "UITexDebug";
            texDesc.acceptMouse = false;
            texDesc.width = 100;
            texDesc.height = 100;
            texDesc.path = "skybox/TropicalSunnyDayBack2048.png";
            texDesc.x = 500.f;
            texDesc.y = 200.f;
            texDesc.show = true;
            TextureDebug tex(texDesc);

            uiFrameObj->frames.push_back(std::make_unique<TextureDebug>(texDesc));
            _tex = (TextureDebug*)uiFrameObj->frames.back().get();
        }

        void AddKeyValue(std::string key, std::string value) {
            _kvList->InsertKey(key, value);
        }

        void SetDebugTextureID(gfx::TextureId texid) {
            _tex->SetTexID(texid);
        }
    };
}
