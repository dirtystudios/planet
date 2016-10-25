#pragma once
#include "UIFrame.h"
#include "FontDesc.h"
#include <deque>
#include <vector>
#include <string>

namespace ui {
    class TextList : public UIFrame {
    public:
        struct TextListDesc : UIFrameDesc {
            //bool scrollable; // currently useless
            uint32_t maxLines{10};
            FontDesc font;
        };

    private:
        std::deque<std::string> m_contents;
        // cause euge said to cache things
        std::vector<std::string> m_contentsCache;
        bool m_cacheDirty;

        TextListDesc m_textListDesc;
    public:
        TextList(TextListDesc textListDesc);
        TextList(TextListDesc textListDesc, BaseScriptHandler* scriptHandler);

        FontDesc GetFontDesc() const { return m_textListDesc.font; }
        std::vector<std::string> GetTextList() const { return m_contentsCache; }
        glm::vec3 GetColor() const { return m_textListDesc.font.color; }
        uint32_t GetMaxLines() const { return m_textListDesc.maxLines; }

        void InsertTextLine(std::string line);

        // Called by UIManager
        void DoUpdate(float ms) {};

    private: 
        void UpdateCache();
    };
}