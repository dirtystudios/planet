#pragma once
#include "RenderDevice.h"
#include "Config.h"
#include "Helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"

class UIRenderer : public Renderer {
private:
    graphics::ShaderParamId _projectionParam;
    graphics::ShaderParamId _bgColorParam;
    graphics::ShaderParamId _borderColorParam;
    graphics::ShaderParamId _borderSizeParam;
    graphics::PipelineStateId _defaultPS;
    graphics::BufferId _vertexBuffer;

public:
    void OnInit() {
        graphics::PipelineStateDesc psd;
        psd.vertexShader               = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "ui");
        psd.pixelShader                = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "ui");
        graphics::VertexLayoutDesc vld = {{{
            graphics::VertexAttributeType::Float4, graphics::VertexAttributeUsage::Position,
        }}};

        psd.vertexLayout            = GetVertexLayoutCache()->Get(vld);
        psd.topology                = graphics::PrimitiveType::Triangles;
        psd.blendState.enable       = true;
        psd.blendState.srcRgbFunc   = graphics::BlendFunc::SrcAlpha;
        psd.blendState.srcAlphaFunc = psd.blendState.srcRgbFunc;
        psd.blendState.dstRgbFunc   = graphics::BlendFunc::OneMinusSrcAlpha;
        psd.blendState.dstAlphaFunc = psd.blendState.dstRgbFunc;

        _defaultPS = GetPipelineStateCache()->Get(psd);
        _projectionParam =
            GetRenderDevice()->CreateShaderParam(psd.vertexShader, "projection", graphics::ParamType::Float4x4);
        _bgColorParam = GetRenderDevice()->CreateShaderParam(psd.pixelShader, "bgColor", graphics::ParamType::Float4);
        _borderColorParam =
            GetRenderDevice()->CreateShaderParam(psd.pixelShader, "borderColor", graphics::ParamType::Float4);
        _borderSizeParam =
            GetRenderDevice()->CreateShaderParam(psd.pixelShader, "borderSize", graphics::ParamType::Float2);

        _vertexBuffer =
            GetRenderDevice()->CreateBuffer(graphics::BufferType::VertexBuffer, 0, 0, graphics::BufferUsage::Static);

        assert(_vertexBuffer);
        assert(_defaultPS);
        assert(_projectionParam);
        assert(_bgColorParam);
        assert(_borderColorParam);
        assert(_borderSizeParam);
    }

    ~UIRenderer() {
        // TODO
    }

    RenderObj* Register(SimObj* simObj) final { return nullptr; }

    void Unregister(RenderObj* renderObj) final {}

    void Submit(RenderQueue* renderQueue, RenderView* renderView) final {}

    // void RenderFrame(float x, float y, uint32_t width, uint32_t height) {

    //     float w = (float)width;
    //     float h = (float)height;

    //     glm::mat4 projection = glm::ortho(0.0f, m_winWidth, 0.0f, m_winHeight);

    //     m_renderDevice->SetVertexShader(m_shaders[0]);
    //     m_renderDevice->SetPixelShader(m_shaders[1]);

    //     m_renderDevice->SetShaderParameter(m_shaders[0], graphics::ParamType::Float4x4, "projection", &projection);

    //     float bgColor[4] = { 0.f, 0.f, 0.f, 0.5f };
    //     float borderColor[4] = { 0.2f, 0.2f, 0.2f, 0.8f};

    //     glm::vec2 borderWidth = { 2.f, 2.f };
    //     glm::vec2 pixelSize = { 1.f / w, 1.f / h };
    //     glm::vec2 borderSize = pixelSize * borderWidth;

    //     m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float4, "bgColor", &bgColor);
    //     m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float4, "borderColor", &borderColor);
    //     m_renderDevice->SetShaderParameter(m_shaders[1], graphics::ParamType::Float2, "borderSize", &borderSize);

    //     m_renderDevice->SetVertexBuffer(m_vertexBuffer);

    //     float xpos = x;
    //     float ypos = y;

    //     float vertices[6][4] = {
    //         {xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

    //         {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

    //     m_renderDevice->UpdateVertexBuffer(m_vertexBuffer, vertices, sizeof(vertices));

    //     m_renderDevice->DrawPrimitive(graphics::PrimitiveType::Triangles, 0, 6);
    // }
};
