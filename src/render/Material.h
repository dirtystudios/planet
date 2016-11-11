#pragma once

#include "ResourceTypes.h"

#include "MaterialData.h"
#include <vector>

struct Material {
	std::vector<MaterialData> matData;
	gfx::ShaderId pixelShader{ 0 };
	/*
    gfx::ShaderId vertexShader { 0 };
    gfx::BufferId constants { 0 };
    gfx::TextureId diffuseMap { 0 };
    gfx::TextureId specularMap { 0 };*/
};
