#pragma once

#include "ResourceTypes.h"

#include "ConstantBuffer.h"
#include "Config.h"
#include "Image.h"
#include "RenderDevice.h"
#include "StateGroupEncoder.h"
#include "MaterialData.h"
#include <vector>
#include <string>
#include <memory>

struct MaterialConstants {
    glm::vec3 ka;
    float padding0;
    glm::vec3 kd;
    float padding1;
    glm::vec3 ks;
    float padding2;
    glm::vec3 ke;
    float ns;
};

class MeshMaterial {
private:
    gfx::TextureId textureid{ 0 };
    gfx::ShaderId pixelShader{ 0 };
    ConstantBuffer* matConstants{ nullptr };
    std::string matName{""};

    std::unique_ptr<const gfx::StateGroup> _stateGroup;

public:
    MeshMaterial(gfx::RenderDevice* device, const MaterialData& matData, gfx::ShaderId ps) {

        std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
        if (!fs::IsPathDirectory(assetDirPath)) {
            LOG_E("Invalid Directory Path given for AssetDirectory.");
        }
        if (matData.diffuseData.data != nullptr) {
            textureid = device->CreateTexture2D(matData.diffuseData.pixelFormat == dimg::PixelFormat::RGBA8Unorm ? gfx::PixelFormat::RGBA8Unorm : gfx::PixelFormat::RGB8Unorm, matData.diffuseData.width, matData.diffuseData.height, matData.diffuseData.data, matData.diffuseMap);
        }
        else {
            // just log for now
            Log::msg(Log::Level::Debug, "MeshMatLoader", "No diffuse map specified, using white");
            // euge said to use a 16x16 white texture
            // and yes, this *is* a new texture everytime
            std::vector<uint32_t> whiteImageData(16 * 16, 0xFFFFFFFF);
            textureid = device->CreateTexture2D(gfx::PixelFormat::RGBA8Unorm, 16, 16, whiteImageData.data(), "MeshMatDiffuseWhite");
        }
        assert(matData.specularMap == "");

        gfx::BufferDesc desc = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ConstantBufferBit, sizeof(MaterialConstants), "MeshMatContants_" + matData.name);
        gfx::BufferId buffer = device->AllocateBuffer(desc);
        matConstants = new ConstantBuffer(buffer, device);
        
        MaterialConstants* materialBuffer = matConstants->Map<MaterialConstants>();
        materialBuffer->ka = matData.ka;
        materialBuffer->kd = matData.kd;
        materialBuffer->ke = matData.ke;
        materialBuffer->ks = matData.ks;
        materialBuffer->ns = matData.ns;
        matConstants->Unmap();

        gfx::StateGroupEncoder encoder;
        encoder.Begin();
        encoder.BindResource(matConstants->GetBinding(2));
        encoder.SetPixelShader(ps);
        encoder.BindTexture(0, textureid, gfx::ShaderStageFlags::PixelBit);
        _stateGroup.reset(encoder.End());
    }

    const gfx::StateGroup* stateGroup() { return _stateGroup.get(); }
};
