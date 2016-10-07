#include "UIDomTree.h"

using namespace dm;

namespace ui {
    void UIDomTree::SetRoot(UIFrame* frame) {
        // dont call this more than once
        if (root) {
            LOG_E("[UIDomTree] - dont call setroot multiple times");
            return;
        }
        root = CreateNode(frame);
        lastInserted = root;
    }

    void UIDomTree::InsertFrame(UIFrame* frame) {
        if (frame->GetParent() != lastInserted->frame) {
            LOG_E("[UIDomTree] - you didn't say the magic word");
            return;
        }
        UIDomNode* node = CreateNode(frame);

        lastInserted->children.emplace_back(node);
        node->parentNode = lastInserted;
        lastInserted = node;
    }

    UIDomTree::UIDomNode* UIDomTree::CreateNode(UIFrame* frame) {
        UIDomNode* node = new UIDomNode();
        node->frame = frame;

        FrameType type = frame->GetFrameType();
        FrameScale scaled = frame->GetScaledSize(m_viewport);
        switch (type) {
        case FrameType::EDITBOX: {
            EditBox*  ebox = dynamic_cast<EditBox*>(frame);
            node->textRO.reset(new TextRenderObj(ebox->GetText(), scaled.x, scaled.y, scaled.z, ebox->GetColor()));
            m_textRenderer->Register(node->textRO.get());
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->GetFrameDesc().show));
            m_uiRenderer->Register(node->frameRO.get());
            break;

        }
        case FrameType::LABEL: {
            // for now label type is always shown
            Label*    label = dynamic_cast<Label*>(frame);
            node->textRO.reset(new TextRenderObj(label->GetText(), scaled.x, scaled.y, scaled.z, label->GetColor()));
            m_textRenderer->Register(node->textRO.get());
            break;
        }
        default:
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->GetFrameDesc().show));
            m_uiRenderer->Register(node->frameRO.get());
            break;
        }

        return node;
    }

    UIFrame* UIDomTree::HitTest(float x, float y) {
        if (anchor != glm::vec3(0.f, 0.f, 0.f) || rotation != glm::vec3(0.f, 0.f, 0.f))
            return nullptr;

        FrameScale scaledFrame = root->frame->GetScaledSize(m_viewport);
        Rect2Df rootRect = Rect2Df(glm::vec2(0.f, 0.f), glm::vec2(m_viewport.width, m_viewport.height));

        return HitTestNode(root, x, y, rootRect);
    }

    UIFrame* UIDomTree::HitTestNode(UIDomNode* node, float x, float y, const Rect2Df& parentRect) {
        if (!node->frame->IsShown())
            return nullptr;

        // make new rect to represent current frame
        FrameScale scaledFrame = node->frame->GetScaledSize(m_viewport);
        glm::vec2 bl = glm::vec2(parentRect.bl().x + scaledFrame.x, parentRect.bl().y + scaledFrame.y);
        Rect2Df curRect = Rect2Df(bl, glm::vec2(bl.x + scaledFrame.width, bl.y + scaledFrame.height));

        // is it inside current frame?
        if (x > curRect.bl().x && x < curRect.tr().x &&
            y > curRect.bl().y && y < curRect.tr().y) {
            
            // sweet, k see if any children are a more specific match...
            //  also this *will* cause chaos if theres a weird ass tree layout with multiple layered ontop of each other
            // todo: layer heiracrchy
            for (auto& child : node->children) {
                UIFrame* test = HitTestNode(child, x, y, curRect);
                if (test)
                    return test;
            }
            // no children want it? k take it if we can
            if (node->frame->GetFrameDesc().acceptMouse)
                return node->frame;
        }
        return nullptr;
    }

    dm::Rect2Df UIDomTree::GetRenderedSize(UIFrame* frame) {
        UIDomNode* node = GetNode(root, frame);
        if (node) {
            UIFrameRenderObj* ro = node->frameRO.get();
            if (ro) {
                FrameScale scaled = node->frame->GetScaledSize(m_viewport);
                glm::vec2 bl = glm::vec2(ro->x(), ro->y());
                Rect2Df curRect = Rect2Df(bl, glm::vec2(bl.x + scaled.width, bl.y + scaled.height));
                return curRect;
            }
        }
        return Rect2Df(glm::vec2(0.f, 0.f), glm::vec2(0.f, 0.f));
    }

    UIDomTree::UIDomNode* UIDomTree::GetNode(UIDomNode* node, UIFrame* frame) {
        if (node->frame == frame)
            return node;
        else {
            for (UIDomNode* child : node->children) {
                UIDomNode* test = GetNode(child, frame);
                if (test)
                    return test;
            }       
        }
        return nullptr;
    }

    void UIDomTree::RenderTree(const glm::vec3& anch, const glm::vec3& rot, bool renderCursor) {
        RenderNodes(root, anch, rot, true, renderCursor);
    }

    void UIDomTree::RenderNodes(UIDomNode* node, const glm::vec3& anch, const glm::vec3& rot, bool shown, bool renderCursor) {
        glm::vec3 newPos = { 0.f, 0.f, 0.f };
        glm::vec3 newRot = {0.f, 0.f, 0.f};

        bool shouldRender = shown && node->frame->GetFrameDesc().show;

        if (shouldRender) {
            FrameScale scaled = node->frame->GetScaledSize(m_viewport);
            glm::vec3 descPos = { scaled.x, scaled.y, scaled.z };
            newPos = anch + descPos;
            newRot = rot + scaled.rot;

            if (node->frameRO.get()) {
                node->frameRO.get()->x(newPos.x);
                node->frameRO.get()->y(newPos.y);
                node->frameRO.get()->z(newPos.z);
                node->frameRO.get()->rot(newRot);
            }
            if (node->textRO.get()) {
                node->textRO.get()->x(newPos.x);
                node->textRO.get()->y(newPos.y);
                node->textRO.get()->z(newPos.z);
            }
        }

        if (node->frameRO.get())
            node->frameRO.get()->isRendered(shouldRender);
        if (node->textRO.get()) {
            node->textRO.get()->cursorEnabled(false);
            if (!shouldRender)
                node->textRO.get()->text("");
            else {
                if (node->frame->GetFrameType() == FrameType::LABEL)
                    node->textRO.get()->text(((Label*)node->frame)->GetText());
                else
                    node->textRO.get()->text(((EditBox*)node->frame)->GetText());
            }
        }

        if (node->frame == m_focused) {
            if (node->frame->GetFrameType() == FrameType::EDITBOX) {
                node->textRO.get()->cursorPos(((EditBox*)node->frame)->GetCursor());
                node->textRO.get()->cursorEnabled(renderCursor);
            }
        }

        for (auto& child : node->children) {
            RenderNodes(child, newPos, newRot, shouldRender, renderCursor);
        }
    }
}
