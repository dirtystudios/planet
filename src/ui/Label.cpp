#include "Label.h"

namespace ui {
    Label::Label(LabelDesc labelDesc) 
        : UIFrame(labelDesc), m_labelDesc(labelDesc) {
        m_frameType = FrameType::LABEL;
        m_contents = labelDesc.text;
    }

    Label::Label(LabelDesc labelDesc, ScriptHandler* scriptHandler) 
        : UIFrame(labelDesc, scriptHandler), m_labelDesc(labelDesc) {
        m_frameType = FrameType::LABEL;
        m_contents = labelDesc.text;
    }

    std::string Label::GetText() const {
        return m_contents;
    }
    
    float* Label::GetColor() {
        return m_labelDesc.font.color;
    }
}