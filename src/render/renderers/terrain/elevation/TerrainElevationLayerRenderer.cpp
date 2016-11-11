#include "TerrainElevationLayerRenderer.h"
#include "DrawItemEncoder.h"
#include "StateGroupEncoder.h"

struct TileConstants {
    glm::mat4 world;
    uint32_t  heightmapIndex;
    uint32_t  lod;
    //    glm::mat4 uvTransform;
    //    glm::vec2 heightmapUVRegionBL;
    //    glm::vec2 heightmapUVRegionTR;
    //    uint32_t normalmapIndex;
    //    glm::vec2 normalmapUVRegionBR;
    //    glm::vec2 normalmapUVRegionTR;
};

void TerrainElevationLayerRenderer::OnInit() {
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "terrain"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "terrain"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3fNormal3fTex2f());
    _base.reset(encoder.End());
}

// class ElevationDataTile {
// public:
//    TerrainTileKey key;
//    double size;
//};
//
// class NormalDataTile {
// public:
//    TerrainTileKey key;
//    double size;
//};
//
// template <class T>
// class DataTileSampler {
// public:
//
//    // returns best available tile
//    virtual T* findTile(const TerrainTileKey& key);
//    virtual glm::vec2 tileResolution() const;
//};
//
// DataTileSampler<ElevationDataTile>* elevationSampler;
// DataTileSampler<NormalDataTile>* normalSampler;

// dm::Rect2Df mapRegion(const TerrainTileKey& fromKey, double fromSize, const TerrainTileKey& toKey, double toSize) {
//
//}
//
// MeshGeometry* getMesh(const glm::vec2& resolution) {
//
//}

void TerrainElevationLayerRenderer::Submit(RenderQueue* renderQueue, const FrameView* view, const std::vector<const TerrainQuadNode*>& selectedQuads) {
    for (const TerrainQuadNode* node : selectedQuads) {
        ElevationDataTile* tile = _elevationSampler->FindTile(node->key);
        if (tile == nullptr) {
            continue;
        }

        if (tile->perTileConstants == nullptr) {
            tile->perTileConstants = services()->constantBufferManager()->GetConstantBuffer(sizeof(TileConstants));
        }

        uint32_t lodDiff = tile->key.lod - node->key.lod;
        float    scale   = dm::pow(2, lodDiff);

        TileConstants* constants  = tile->perTileConstants->Map<TileConstants>();
        constants->world          = node->worldMatrix();
        constants->heightmapIndex = tile->gpuData->slotIndex;
        constants->lod            = tile->key.lod;
        //        constants->uvTransform = lodDiff == 0 ? glm::scale(glm::mat4(), glm::vec3(scale, scale, 1));
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
            assert(tile->geometry->drawCalls().size() == 1);
            gfx::DrawItemEncoder encoder;
            tile->drawItem.reset(encoder.Encode(device(), tile->geometry->drawCalls()[0], {tile->stateGroup.get()}));
        }

        renderQueue->AddDrawItem(50, tile->drawItem.get());

        //        glm::vec2 resolution = elevationSampler->tileResolution();
        //
        //        ElevationDataTile* elevationTile = elevationSampler->findTile(node->key);
        //        NormalDataTile* normalTile = normalSampler->findTile(node->key);
        //
        //        dm::Rect2Df elevationUVRegion = mapRegion(node->key, node->size, elevationTile->key, elevationTile->size);
        //        dm::Rect2Df normalUVRegion = mapRegion(node->key, node->size, normalTile->key, normalTile->size);
        //
        //        ConstantBuffer* constants;
        //        TileConstants* tileConstants = constants->Map<TileConstants>();
        //        tileConstants->heightmapIndex = 0;
        //        tileConstants->normalmapIndex = 0;
        //        tileConstants->heightmapUVRegionBL = elevationUVRegion.bl();
        //        tileConstants->heightmapUVRegionTR = elevationUVRegion.tr();
        //        tileConstants->normalmapUVRegionBR = normalUVRegion.bl();
        //        tileConstants->normalmapUVRegionTR = normalUVRegion.tr();
        //        tileConstants->world = node->worldMatrix();
        //        constants->Unmap();
        //
        //        MeshGeometry* geometry = getMesh(resolution);
        //
        //        gfx::StateGroupEncoder encoder;
        //        encoder.Begin();
        //        const gfx::StateGroup* sg = encoder.End();
        //
        //        const gfx::DrawItem* item = gfx::DrawItemEncoder::Encode(device(), geometry->drawCall(), &sg, 1);
        //        renderQueue->AddDrawItem(0, item);
    }
    //    for (TerrainDataTile* baseTile : selectedDataTiles) {
    //        dg_assert_nm(baseTile->layerType == TerrainLayerType::Heightmap);
    //
    //
    //        TerrainElevationTile* tile = reinterpret_cast<TerrainElevationTile*>(baseTile);
    //        if(tile->perTileConstants == nullptr) {
    //            tile->perTileConstants = services()->constantBufferManager()->GetConstantBuffer(sizeof(TileConstants));
    //
    //            TileConstants* constants = tile->perTileConstants->Map<TileConstants>();
    //            constants->world = tile->transform;
    //            constants->heightmapIndex = tile->gpuSlot->slotIndex;
    //            tile->perTileConstants->Unmap();
    //        }
    //
    //        if(tile->stateGroup == nullptr) {
    //            std::unique_ptr<const gfx::StateGroup> temp;
    //
    //            gfx::StateGroupEncoder encoder;
    //            encoder.Begin(renderQueue->defaults);
    //            encoder.BindResource(tile->perTileConstants->GetBinding(1));
    //            encoder.BindTexture(0, tile->gpuSlot->texture, gfx::ShaderStageFlags::AllStages);
    //            temp.reset(encoder.End());
    //
    //            tile->stateGroup.reset(encoder.Merge({_base.get(), tile->geometry->stateGroup(), temp.get()}));
    //        }
    //
    //        if(tile->drawItem == nullptr) {
    //            gfx::DrawItemEncoder encoder;
    //            tile->drawItem.reset(encoder.Encode(device(), tile->geometry->drawCall(), {tile->stateGroup.get()}));
    //        }
    //
    //        renderQueue->AddDrawItem(0, tile->drawItem.get());
    //    }
}
