#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"
#include "StateGroupEncoder.h"
#include "MeshGeometry.h"
#include "DrawItemEncoder.h"
#include "Image.h"
#include "Config.h"

struct MeshConstants {
    glm::mat4 world;
};

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

struct tempVert {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 tex;
};
std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");

MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "blinn"));
    baseStateGroup.reset(encoder.End());

	MeshRenderData renderData;

    renderData.cb1  = services()->constantBufferManager()->GetConstantBuffer(sizeof(MeshConstants));
    renderData.cb2  = services()->constantBufferManager()->GetConstantBuffer(sizeof(MaterialConstants));
    renderData.mesh = services()->meshCache()->Get("roxas/roxas.obj");
    renderData.mat  = services()->materialCache()->Get("roxas/roxas.obj", services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "blinn"));

	meshRenderData.emplace_back(std::move(renderData));
}

void MeshRenderer::Submit(RenderQueue* renderQueue, const FrameView* renderView) {
	_drawItems.clear();
    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(2, 2, 2));
	for (MeshRenderData& renderData : meshRenderData) {
		assert(renderData.cb1);
		assert(renderData.cb2);
		assert(renderData.mat);
		assert(renderData.mesh);

		MeshConstants* meshBuffer = renderData.cb1->Map<MeshConstants>();
		meshBuffer->world = world;
		renderData.cb1->Unmap();

		MaterialConstants* materialBuffer = renderData.cb2->Map<MaterialConstants>();
		materialBuffer->ka = renderData.mat->matData[1].ka;
		materialBuffer->kd = renderData.mat->matData[1].kd;
		materialBuffer->ke = renderData.mat->matData[1].ke;
		materialBuffer->ks = renderData.mat->matData[1].ks;
		materialBuffer->ns = renderData.mat->matData[1].ns;
		renderData.cb2->Unmap();

		if (renderData.textureid == 0) {
			dimg::Image imageData;
			std::string path = assetDirPath  + "roxas/" + renderData.mat->matData[1].diffuseMap;
			dimg::LoadImageFromFile(path.c_str(), &imageData);
			renderData.textureid = device()->CreateTexture2D(imageData.pixelFormat == dimg::PixelFormat::RGBA8Unorm ? gfx::PixelFormat::RGBA8Unorm : gfx::PixelFormat::RGB8Unorm, imageData.width, imageData.height, imageData.data);
		}

        if (renderData.meshGeometry.get() == nullptr) {
            std::vector<MeshGeometryData> tempGeomData;
            for (const MeshPart& part : renderData.mesh->GetParts()) {
                tempGeomData.push_back(part.geometryData);
            }
            renderData.meshGeometry.reset(new MeshGeometry(device(), tempGeomData));
        }

		if (renderData.stateGroup.get() == nullptr) {
			gfx::StateGroupEncoder encoder;
			encoder.Begin(baseStateGroup.get());
			encoder.BindResource(renderData.cb1->GetBinding(1));
			encoder.BindResource(renderData.cb2->GetBinding(2));
			encoder.SetPixelShader(renderData.mat->pixelShader);
			encoder.BindTexture(0, renderData.textureid, gfx::ShaderStageFlags::PixelBit);
			renderData.stateGroup.reset(encoder.End());
		}

        for (const gfx::DrawCall& dc : renderData.meshGeometry->drawCalls()) {
            gfx::DrawItemEncoder encoder;

            std::unique_ptr<const gfx::DrawItem> drawItem;

            std::vector<const gfx::StateGroup*> groups = { renderData.stateGroup.get(), renderData.meshGeometry->stateGroup(), renderQueue->defaults };

            drawItem.reset(encoder.Encode(device(), dc, groups.data(), groups.size()));

            _drawItems.emplace_back(std::move(drawItem));
        }

		for (auto &drawItem : _drawItems) {
			renderQueue->AddDrawItem(0, drawItem.get());
		}
	}
}
