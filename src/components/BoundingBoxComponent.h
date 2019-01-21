#pragma once

#include "Component.h"
#include "ComponentType.h"
#include "BoundingBox.h"
#include <vector>

class BoundingBoxComponent : public Component {
public:
    std::vector<dm::BoundingBox> bboxs;
    static constexpr ComponentType type() { return ComponentType::BoundingBox; }
};