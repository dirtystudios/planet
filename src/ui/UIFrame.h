#pragma once
#include <cstdint>
#include <cstring>
#include <string>

namespace ui {

    enum class FrameType : uint32_t {
        UIFRAME = 0,
        EDITBOX,
        COUNT,
    };

    class UIFrame {
    public:
        struct UIFrameDesc  {
            UIFrameDesc()
                : name(""), x(0), y(0), width(0), height(0), parent(0), shown(true) {}

            std::string name;
            float x;
            float y;
            uint32_t width;
            uint32_t height;
            bool shown;
            UIFrame *parent;
        };
    protected:
        UIFrameDesc m_frameDesc;
        FrameType m_frameType;
        bool m_acceptsInput;
    public:
        UIFrame(UIFrameDesc frameDesc, bool enableInput) :
            m_frameDesc(frameDesc),
            m_acceptsInput(enableInput),
            m_frameType(FrameType::UIFRAME) {};
        UIFrameDesc* GetFrameDesc() { return &m_frameDesc; };
        FrameType GetFrameType() { return m_frameType; };
        bool AcceptingInput() { return (m_acceptsInput && m_frameDesc.shown); };
        void Show() { m_frameDesc.shown = true; };
        void Hide() { m_frameDesc.shown = false; };
        bool IsShown() { return m_frameDesc.shown; };
        UIFrame* GetParent() { return m_frameDesc.parent; };
		virtual void OnClick() {};
        virtual void DoUpdate(float ms) {};
    };
}
