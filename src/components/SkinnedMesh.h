#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Component.h"
#include "ComponentType.h"

class SkinnedMesh: public Component {
public:
    MeshPtr mesh;
    MaterialPtr mat;
    float scale{ 1.f };
    static constexpr ComponentType type() { return ComponentType::SkinnedMesh; }
};