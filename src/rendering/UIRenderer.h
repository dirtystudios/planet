#pragma once
#include "RenderDevice.h"
#include "Config.h"
#include "Helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ui {
    class UIRenderer {
    private:
        graphics::RenderDevice* m_renderDevice;
        graphics::ShaderHandle m_shaders[2];
        graphics::VertexBufferHandle m_vertexBuffer;
        float m_winWidth, m_winHeight;
    public:
        UIRenderer(graphics::RenderDevice* renderDevice, float windowWidth, float windowHeight) : m_renderDevice(renderDevice), m_winWidth(windowWidth), m_winHeight(windowHeight) {
            std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
            if (!fs::IsPathDirectory(shaderDirPath)) {
                LOG_E("%s", "Invalid Directory Path given for ShaderDirectory. Attempting default.");
                shaderDirPath = fs::AppendPathProcessDir("/shaders");
            }
            std::string vs_contents = ReadFileContents(shaderDirPath + "/" + m_renderDevice->DeviceConfig.DeviceAbbreviation + "/ui_vs" + m_renderDevice->DeviceConfig.ShaderExtension);
            const char* vs_src = vs_contents.c_str();

            std::string fs_contents = ReadFileContents(shaderDirPath + "/" + m_renderDevice->DeviceConfig.DeviceAbbreviation + "/ui_ps" + m_renderDevice->DeviceConfig.ShaderExtension);
            const char* fs_src = fs_contents.c_str();

            m_shaders[0] = m_renderDevice->CreateShader(graphics::ShaderType::VERTEX_SHADER, &vs_src);
            m_shaders[1] = m_renderDevice->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, &fs_src);

            graphics::VertLayout layout;
            layout.Add(graphics::ParamType::Float4);
            m_vertexBuffer = m_renderDevice->CreateVertexBuffer(layout, 0, graphics::SizeofParam(graphics::ParamType::Float4) * 6, graphics::BufferUsage::DYNAMIC);
        }

        void SetRenderWindowSize(uint32_t width, uint32_t height) {
            m_winWidth = (float)width;
            m_winHeight = (float)height;
        }

        void RenderFrame(float x, float y, uint32_t width, uint32_t height) {
            graphics::BlendState blend_state;
            blend_state.enable = true;
            blend_state.src_rgb_func = graphics::BlendFunc::SRC_ALPHA;
            blend_state.src_alpha_func = blend_state.src_rgb_func;
            blend_state.dst_rgb_func = graphics::BlendFunc::ONE_MINUS_SRC_ALPHA;
            blend_state.dst_alpha_func = blend_state.dst_rgb_func;
            m_renderDevice->SetBlendState(blend_state);

            float w = (float)width;
            float h = (float)height;

            glm::mat4 projection = glm::ortho(0.0f, m_winWidth, 0.0f,  m_winHeight);

            m_renderDevice->SetVertexShader(m_shaders[0]);
            m_renderDevice->SetPixelShader(m_shaders[1]);
            
            m_renderDevice->SetShaderParameter(m_shaders[0], graphics::ParamType::Float4x4, "projection", &projection);

            float bgColor[4] = { 0.f, 0.f, 0.f, 0.5f };
            float borderColor[4] = { 0.2f, 0.2f, 0.2f, 0.8f};

            glm::vec2 borderWidth = { 2.f, 2.f };
            glm::vec2 pixelSize = { 1.f / w, 1.f / h };
            glm::vec2 borderSize = pixelSize * borderWidth;

            m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float4, "bgColor", &bgColor);
            m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float4, "borderColor", &borderColor);
            m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float2, "borderSize", &borderSize);

            m_renderDevice->SetVertexBuffer(m_vertexBuffer);

            float xpos = x;
            float ypos = y;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos    ,   0.0, 1.0 },
                { xpos + w, ypos    ,   1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos    ,   1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
            };

            m_renderDevice->UpdateVertexBuffer(m_vertexBuffer, vertices, sizeof(vertices));

            m_renderDevice->DrawPrimitive(graphics::PrimitiveType::TRIANGLES, 0, 6);
        }
    };
}