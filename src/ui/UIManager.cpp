#pragma once
#include "UIManager.h"

namespace ui {
    void UIManager::AddFrame(UIFrame* uiFrame) {
        m_uiFrames.emplace_back(uiFrame);
    }

    void UIManager::DoUpdate(float dt) {
        // todo: optimize, this is lazily done for now
        for (UIFrame* uiFrame : m_uiFrames) {
            if (uiFrame->IsShown()) {
                // should do some sort of cache or something for this, but w/e
                UIFrame::UIFrameDesc* frameDesc = uiFrame->GetFrameDesc();
                m_uiRenderer->RenderFrame(frameDesc->x, frameDesc->y,
                    frameDesc->width, frameDesc->height);
            }
        }
    }
}