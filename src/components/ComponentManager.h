#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Viewport.h"

#include <array>
#include <map>
#include <memory>

class ComponentManager {
public:
    virtual void UpdateViewport(const Viewport& vp) = 0;
    virtual void DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, 1024u>*>& components, float ms) = 0;
};
