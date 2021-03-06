#pragma once 

#include "Component.h"
#include "UIFrame.h"
#include "ComponentType.h"
#include <vector>
#include <memory>

class UI : public Component {
public:
    std::vector<std::unique_ptr<ui::UIFrame>> frames;
    bool isWorldFrame{ false };

    static constexpr ComponentType type() { return ComponentType::UI; }
};