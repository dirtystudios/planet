#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Component.h"

class SkinnedMesh: public Component {
public:
    MeshPtr mesh;
    MaterialPtr mat;
    float scale{ 1.f };
};