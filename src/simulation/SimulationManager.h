#pragma once

#include "Spatial.h"
#include "UI.h"
#include "SkinnedMesh.h"
#include "AnimationComponent.h"
#include "ComponentManager.h"

#include <vector>
#include <array>
#include <memory>

// this is manually set in componentManager also, yey hardcodes
constexpr uint32_t MAX_SIM_OBJECTS = 1024;

class SimObj;
class SimulationManager {
private:
    friend class SimObj;
    std::array<std::unique_ptr<Spatial>, MAX_SIM_OBJECTS> spatials;
    std::array<std::unique_ptr<UI>, MAX_SIM_OBJECTS> uis;
    std::array<std::unique_ptr<SkinnedMesh>, MAX_SIM_OBJECTS> skinnedMeshs;
    std::array<std::unique_ptr<AnimationComponent>, MAX_SIM_OBJECTS> animations;

    std::array<std::unique_ptr<SimObj>, MAX_SIM_OBJECTS> _simObjs;

    std::vector<std::pair<std::unique_ptr<ComponentManager>, std::vector<ComponentType>>> managers;

    uint64_t nextObjId{0};
public:
    SimulationManager();
    ~SimulationManager();
    template <class T, typename... Args>
    void RegisterManager(const std::vector<ComponentType>& types, Args&&... args) {
        managers.emplace_back(std::make_unique<T>(std::forward<Args>(args)...), types);
    }

    SimObj* CreateSimObj();
    void DoUpdate(float ms);

    // todo: this is mostly a hack for ui manager
    void UpdateViewport(const Viewport& vp);

private:
    bool HasComponent(uint64_t key, ComponentType t);
    Component* AddComponent(uint64_t key, ComponentType t);
    void RemoveComponent(uint64_t key, ComponentType t);

    template <typename T>
    T* GetComponent(uint64_t key, ComponentType t) {
        dg_assert_nm(key < MAX_SIM_OBJECTS);
        switch(t) {
            case ComponentType::Spatial:     return spatials[key];
            case ComponentType::UI:          return uis[key];
            case ComponentType::SkinnedMesh: return skinnedMeshs[key];
            case ComponentType::Animation:   return animations[key];
            default:
                dg_assert_fail("type not accounted for.");
            break;
        }
        return nullptr;
    }
};