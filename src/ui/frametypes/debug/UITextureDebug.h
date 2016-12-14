#pragma once
#include "UITexture.h"
#include "RenderDevice.h"

namespace ui {
    class TextureDebug : public Texture {
    private: 
        gfx::TextureId m_texId;

    public:
        TextureDebug(TextureDesc textureDesc) : Texture(textureDesc) {
            m_frameType = FrameType::TEXTURE;
        }
        void SetTexID(gfx::TextureId texid) { m_texId = texid; };
        gfx::TextureId GetTexID() const { return m_texId; }
    };
}