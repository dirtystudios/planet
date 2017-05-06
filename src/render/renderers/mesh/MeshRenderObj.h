#pragma once

#include <memory>
#include <vector>
#include <string>
#include "DrawItem.h"
#include "MeshGeometry.h"
#include "MeshMaterial.h"
#include "Material.h"
#include "Mesh.h"
#include "RenderObj.h"

class MeshRenderer;

class MeshRenderObj : public RenderObj {
private:
    friend MeshRenderer;

    MaterialPtr mat{ nullptr };
    MeshPtr mesh{ nullptr };
    std::vector<std::unique_ptr<MeshMaterial>> meshMaterial;
    std::vector<std::unique_ptr<MeshGeometry>> meshGeometry;
    ConstantBuffer* perObject{ nullptr };
    std::unique_ptr<const gfx::StateGroup>   stateGroup;

    std::string _meshName, _matName;

public:
    MeshRenderObj(const MeshPtr& mesh, const MaterialPtr& material)
        : RenderObj(RendererType::Mesh), mesh(mesh), mat(material) {};
    ~MeshRenderObj() {}
};
