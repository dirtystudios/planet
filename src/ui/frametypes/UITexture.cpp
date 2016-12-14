#include "UITexture.h"
#include "Image.h"

namespace ui {
    Texture::Texture(TextureDesc textureDesc) 
        : UIFrame(textureDesc), m_textureDesc(textureDesc) {
        m_frameType = FrameType::TEXTURE;
    }

    Texture::Texture(TextureDesc textureDesc, BaseScriptHandler* scriptHandler) 
        : UIFrame(textureDesc, scriptHandler), m_textureDesc(textureDesc) {
        m_frameType = FrameType::TEXTURE;
    }
};