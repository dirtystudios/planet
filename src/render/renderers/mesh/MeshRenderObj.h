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
#include "DMath.h"

class MeshRenderer;

class MeshRenderObj : public RenderObj {
private:
    friend MeshRenderer;

    MaterialPtr mat{ nullptr };
    MeshPtr mesh{ nullptr };
    std::vector<std::unique_ptr<MeshMaterial>> meshMaterial;
    std::vector<std::unique_ptr<MeshGeometry>> meshGeometry;
    std::vector<glm::mat4> _boneOffsets;
    ConstantBuffer* perObject{ nullptr };
    std::unique_ptr<const gfx::StateGroup>   stateGroup;
    
    dm::Transform _transform;

public:
    MeshRenderObj(const MeshPtr& mesh, const MaterialPtr& material)
        : RenderObj(RendererType::Mesh), mesh(mesh), mat(material) {};
    
    
    const std::vector<glm::mat4>& boneOffsets() const { return _boneOffsets; }
    dm::Transform* transform() { return &_transform; };

    void boneOffsets(const std::vector<glm::mat4>& offsets) {
        // todo: ehh... maybe this could be a std::array with a max size for bones instead of a vector.
        // would help out, and with the shader neeeding hardcode of max anyway is probly better
        _boneOffsets = std::vector<glm::mat4>(offsets);
    }
    
    ~MeshRenderObj() {}
};
