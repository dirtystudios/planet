#include "TextList.h"

namespace ui {
    TextList::TextList(TextListDesc textListDesc) 
        : UIFrame(textListDesc), m_textListDesc(textListDesc) {
        m_frameType = FrameType::TEXTLIST;
    }

    TextList::TextList(TextListDesc textListDesc, BaseScriptHandler* scriptHandler)
        : UIFrame(textListDesc, scriptHandler), m_textListDesc(textListDesc) {
        m_frameType = FrameType::TEXTLIST;
    }

    void TextList::UpdateCache() {
        m_contentsCache.clear();
        m_contentsCache.reserve(m_contents.size());
        
        for (std::string line : m_contents) {
            m_contentsCache.push_back(line);
        }
    }

    void TextList::InsertTextLine(std::string line) {
        m_contents.push_front(std::move(line));
        if (m_contents.size() > m_textListDesc.maxLines) {
            m_contents.pop_back();
        }
        UpdateCache();
    }
}