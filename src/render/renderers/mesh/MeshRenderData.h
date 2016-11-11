#pragma once

#include <memory>
#include "DrawItem.h"
#include "MeshGeometry.h"

struct MeshRenderData {
	Material* mat{ nullptr };
	Mesh* mesh{ nullptr };
    std::unique_ptr<MeshGeometry> meshGeometry;
	ConstantBuffer* cb1{ nullptr };
	ConstantBuffer* cb2{ nullptr };
	gfx::TextureId textureid{ 0 };
	std::unique_ptr<const gfx::StateGroup>   stateGroup;
};