#include "TerrainElevationLayerRenderer.h"
#include <set>
#include "DGAssert.h"
#include "DrawItemEncoder.h"
#include "StateGroupEncoder.h"
#include "TerrainQuadTree.h"

struct TileConstants {
    glm::mat4 world;
    uint32_t  heightmapIndex;
    uint32_t  heightmapLod;
    uint32_t  lod;
};

void TerrainElevationLayerRenderer::OnInit() {
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "terrain"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "terrain"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3fNormal3fTex2f());
    _base.reset(encoder.End());
}

void TerrainElevationLayerRenderer::Submit(RenderQueue* renderQueue, const FrameView* view, const std::vector<const TerrainQuadNode*>& selectedQuads) {

    // TODO: fallback tile solution still has issues at high detail but good enough for now

    std::set<TerrainTileKey>        keys;
    std::vector<glm::mat4>          transforms;
    std::vector<ElevationDataTile*> dataTiles;

    // determine which data tiles we want to render.
    for (const TerrainQuadNode* node : selectedQuads) {
        ElevationDataTile* tile = _elevationSampler->FindTile(node->key);
        if (tile == nullptr) {
            continue;
        }

        // FindTile could return a parent key if the desired tile isnt available yet so need to test for dups
        if (keys.count(tile->key) == 0) {
            dataTiles.push_back(tile);
            keys.insert(tile->key);
            transforms.push_back(node->terrain->worldTransformForKey(tile->key));
        }
    }

    for (uint32_t idx = 0; idx < dataTiles.size(); ++idx) {
        ElevationDataTile* tile = dataTiles[idx];

        // dont render children of parents we plan to render
        TerrainTileKey parentKey = getParentKey(tile->key);
        if (keys.count(parentKey) > 0) {
            continue;
        };

        if (tile->perTileConstants == nullptr) {
            tile->perTileConstants = services()->constantBufferManager()->GetConstantBuffer(sizeof(TileConstants));
        }

        TileConstants* constants  = tile->perTileConstants->Map<TileConstants>();
        constants->world          = transforms[idx];
        constants->heightmapIndex = tile->gpuData->slotIndex;
        constants->heightmapLod   = tile->key.lod;
        constants->lod            = tile->key.lod;

        tile->perTileConstants->Unmap();

        if (tile->stateGroup == nullptr) {
            std::unique_ptr<const gfx::StateGroup> temp;

            gfx::StateGroupEncoder encoder;
            encoder.Begin(renderQueue->defaults);
            encoder.BindResource(tile->perTileConstants->GetBinding(1));
            encoder.BindTexture(0, tile->gpuData->texture, gfx::ShaderStageFlags::AllStages);
            temp.reset(encoder.End());

            tile->stateGroup.reset(encoder.Merge({_base.get(), tile->geometry->stateGroup(), temp.get()}));
        }

        if (tile->drawItem == nullptr) {
            // oops, sorry euge
            dg_assert_nm(tile->geometry->drawCalls().size() == 1);
            gfx::DrawItemEncoder encoder;
            tile->drawItem.reset(encoder.Encode(device(), tile->geometry->drawCalls()[0], {tile->stateGroup.get()}));
        }

        renderQueue->AddDrawItem(50, tile->drawItem.get());
    }
}
