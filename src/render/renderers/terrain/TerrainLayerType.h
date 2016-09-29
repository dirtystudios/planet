#pragma once

enum class TerrainLayerType : uint8_t { Heightmap = 0, Normalmap = 1, Count = 2 };

static constexpr uint32_t kTerrainLayerTypeCount = static_cast<uint32_t>(TerrainLayerType::Count);
