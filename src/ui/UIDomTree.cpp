#include "UIDomTree.h"

#include "EditBox.h"
#include "TextList.h"
#include "KeyValueList.h"
#include "Label.h"
#include "Rectangle.h"
#include "UITexture.h"

using namespace dm;

static const std::string uiDomTreeChannel = "UIDomTree";
#define DomTreeLogE(fmt, ...) LOG(Log::Level::Error, uiDomTreeChannel, fmt, ##__VA_ARGS__)

namespace ui {
    void UIDomTree::SetRoot(UIFrame* frame) {
        // dont call this more than once
        if (root) {
            assert(root != 0);
            DomTreeLogE("dont call setroot multiple times");
            return;
        }
        root = CreateNode(frame);
        lastInserted = root;
    }

    void UIDomTree::InsertFrame(UIFrame* frame) {
        if (frame->GetParent() != lastInserted->frame) {
            lastInserted = GetNode(root, frame->GetParent());
            if (lastInserted == nullptr) {
                assert(lastInserted != 0);
                DomTreeLogE("Couldn't find parent node for insertFrame");
                return;
            }
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
            node->textROs.push_back(std::make_unique<TextRenderObj>(ebox->GetText(), scaled.x, scaled.y, scaled.z, ebox->GetColor(), !m_worldFrame));
            m_textRenderer->Register(node->textROs[0].get());
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->IsShown(), !m_worldFrame));
            m_uiRenderer->Register(node->frameRO.get());
            break;

        }
        case FrameType::TEXTLIST: {
            TextList* textList = dynamic_cast<TextList*>(frame);
            int maxLines = textList->GetMaxLines();
            for (int x = 0; x < maxLines; ++x) {
                node->textROs.push_back(std::make_unique<TextRenderObj>("", scaled.x, scaled.y, scaled.z, textList->GetColor(), !m_worldFrame));
                m_textRenderer->Register(node->textROs.back().get());
            }
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->IsShown(), !m_worldFrame));
            m_uiRenderer->Register(node->frameRO.get());
            break;
        }
        case FrameType::LABEL: {
            // for now label type is always shown
            Label*    label = dynamic_cast<Label*>(frame);
            node->textROs.push_back(std::make_unique<TextRenderObj>(label->GetText(), scaled.x, scaled.y, scaled.z, label->GetColor(), !m_worldFrame));
            m_textRenderer->Register(node->textROs[0].get());
            break;
        }

        case FrameType::KEYVALUE: {
            KeyValueList* kvl = dynamic_cast<KeyValueList*>(frame);
            int maxLines = kvl->GetMaxLines();
            for (int x = 0; x < maxLines; ++x) {
                node->textROs.push_back(std::make_unique<TextRenderObj>("", scaled.x, scaled.y, scaled.z, glm::vec3(1.f, 1.f, 1.f), !m_worldFrame));
                m_textRenderer->Register(node->textROs.back().get());
            }
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->IsShown(), !m_worldFrame));
            m_uiRenderer->Register(node->frameRO.get());
            break;
        }

        case FrameType::TEXTURE: {
            Texture* tex = dynamic_cast<Texture*>(frame);
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->IsShown(), !m_worldFrame));
            node->frameRO->texPath(tex->GetTexture());
            m_uiRenderer->Register(node->frameRO.get());
        }
        default:
            node->frameRO.reset(
                new UIFrameRenderObj(scaled.x, scaled.y, scaled.z, scaled.width, scaled.height, scaled.rot, frame->IsShown(), !m_worldFrame));
            m_uiRenderer->Register(node->frameRO.get());
            break;
        }

        return node;
    }

    UIFrame* UIDomTree::HitTest(float x, float y) {
        if (m_anchor != glm::vec3(0.f, 0.f, 0.f) || m_rotation != glm::vec3(0.f, 0.f, 0.f))
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

    void UIDomTree::RenderTree(bool renderCursor) {
        RenderNodes(root, m_anchor, m_rotation, true, renderCursor);
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

            float y = newPos.y;
            for (auto& textRO : node->textROs) {
                if (textRO.get()) {
                    textRO.get()->x(newPos.x + 5.f);
                    textRO.get()->y(y + 5.f);
                    textRO.get()->z(newPos.z);
                    // todo, get lineheight....
                    y += 12.f;
                }
            }
        }

        if (node->frameRO.get())
            node->frameRO.get()->isRendered(shouldRender);

        if (!shouldRender) {
            for (auto& textRO : node->textROs) {
                textRO.get()->text("");
                textRO.get()->cursorEnabled(false);
            }
        }

        else {
            if (node->textROs.size() > 0) {
                if (node->frame->GetFrameType() == FrameType::LABEL)
                    node->textROs[0].get()->text(((Label*)node->frame)->GetText());
                else if (node->frame->GetFrameType() == FrameType::EDITBOX)
                    node->textROs[0].get()->text(((EditBox*)node->frame)->GetText());
                else if (node->frame->GetFrameType() == FrameType::TEXTLIST) {
                    TextList* textList = (TextList*)node->frame;
                    int x = 0;
                    assert(textList->GetMaxLines() == node->textROs.size());
                    for (auto text : textList->GetTextList()) {
                        if (node->textROs[x].get()) {
                            node->textROs[x].get()->text(text);
                        }
                        else
                            assert(false);
                        ++x;
                    }
                }
                else if (node->frame->GetFrameType() == FrameType::KEYVALUE) {
                    KeyValueList* kvl = (KeyValueList*)node->frame;
                    int x = 0;
                    assert(kvl->GetMaxLines() == node->textROs.size());
                    for (auto text : kvl->GetList()) {
                        if (node->textROs[x].get()) {
                            node->textROs[x].get()->text(text);
                        }
                        else
                            assert(false);
                        ++x;
                    }
                }

                else {
                    DomTreeLogE("Unknown frametype that has a textRO");
                    assert(false);
                }
            }

            if (node->frame == m_focused) {
                if (node->frame->GetFrameType() == FrameType::EDITBOX) {
                    node->textROs[0].get()->cursorPos(((EditBox*)node->frame)->GetCursor());
                    node->textROs[0].get()->cursorEnabled(renderCursor);
                }
            }
        }
        for (auto& child : node->children) {
            RenderNodes(child, newPos, newRot, shouldRender, renderCursor);
        }
    }
}
