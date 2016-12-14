#pragma once
#include "UIFrame.h"
#include "Image.h"
#include <string>
#include <memory>

namespace ui {
    class Texture : public UIFrame {
    public:
        struct TextureDesc : UIFrameDesc {
            std::string path{""};
        };

    private:
        std::unique_ptr<dimg::Image> m_image;
        TextureDesc m_textureDesc;

    public:
        Texture(TextureDesc textureDesc);
        Texture(TextureDesc textureDesc, BaseScriptHandler* scriptHandler);

        const std::string GetTexture() const { return m_textureDesc.path; }

        // Called by UIManager
        void DoUpdate(float ms) {};
    };
};