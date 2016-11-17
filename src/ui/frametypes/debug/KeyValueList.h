#pragma once
#include "UIFrame.h"
#include "FontDesc.h"
#include <string>
#include <unordered_map>
#include "DGAssert.h"

namespace ui {
    class KeyValueList : public UIFrame {

        std::unordered_map<std::string, std::string> keyValues;
    public:
        KeyValueList(UIFrameDesc keyValueDesc) : UIFrame(keyValueDesc) {
            m_frameType = FrameType::KEYVALUE;
        }

        void InsertKey(std::string key, std::string value) {
            dg_assert_nm(keyValues.size() <= GetMaxLines());
            auto it = keyValues.insert_or_assign(key, value);
        }

        std::vector<std::string> GetList() {
            std::vector<std::string> rtnVector(keyValues.size());
            int count = 0;
            for (auto& kv : keyValues) {
                rtnVector[count] = kv.first + " -- " + kv.second;
                ++count;
            }
            return rtnVector;
        }

        int GetMaxLines() {
            return 5;
        }

        // Called by UIManager
        void DoUpdate(float ms) {};
    };
}