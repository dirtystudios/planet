#pragma once
#include "TextRenderer.h"
#include "UIRenderer.h"
#include "UIFrame.h"
#include "EditBox.h"
#include "Label.h"
#include "FontDesc.h"
#include "Rectangle.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace ui {

    class UIDomTree {
    private:
        class UIDomNode {
        public:
            UIFrame* frame{nullptr};

            std::unique_ptr<TextRenderObj> textRO;
            std::unique_ptr<UIFrameRenderObj> frameRO;

            UIDomNode* parentNode{nullptr};
            // not using a vector would be better probly
            std::vector<UIDomNode*> children;
        };
        UIDomNode* root;

        // this is *def* not a nasty hack spawned out of lazines
        UIDomNode* lastInserted;

        // anchor pos
        glm::vec3 anchor{0.f, 0.f, 0.f};
        glm::vec3 rotation{0.f, 0.f, 0.f};

        TextRenderer* m_textRenderer;
        UIRenderer* m_uiRenderer;
        Viewport m_viewport;
        UIFrame* m_focused;

    public:
        // note: i dislike sending renderers to this
        UIDomTree(TextRenderer* text, UIRenderer* ui, Viewport viewport) 
            : m_textRenderer(text), m_uiRenderer(ui), m_viewport(viewport) {}

        void SetRoot(UIFrame* frame);

        // currently this *has* to be the child of the last inserted frame, oops
        void InsertFrame(UIFrame* frame);

        // returns frame or null
        // This will instant return null on non last 0 anch or rotation, as it assumes only world frame stuff for now
        UIFrame* HitTest(float x, float y);

        void SetFocus(UIFrame* frame) { m_focused = frame; }
        void RenderTree(const glm::vec3& anch, const glm::vec3& rot, bool renderCursor);
    private:
        UIFrame* HitTestNode(UIDomNode* node, float x, float y, const Rect2Df& parentRect);
        UIDomNode* CreateNode(UIFrame* frame);
        void RenderNodes(UIDomNode* node, const glm::vec3& anch, const glm::vec3& rot, bool shown, bool renderCursor);
    };
}