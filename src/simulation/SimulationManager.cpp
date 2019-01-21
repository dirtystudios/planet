#include "SimulationManager.h"
#include "SimObj.h"

#include <type_traits>

SimulationManager::SimulationManager(EventManager* em)
    : eventManager(em)
{}

SimulationManager::~SimulationManager() = default;

SimObj* SimulationManager::CreateSimObj() {
    dg_assert(nextObjId < MAX_SIM_OBJECTS, "Max simobjects reached");
    _simObjs[nextObjId] = std::make_unique<SimObj>(nextObjId, this);
    return _simObjs[nextObjId++].get();
}

void SimulationManager::UpdateViewport(const Viewport& vp) {
    for (auto& p: managers) {
        p.first->UpdateViewport(vp);
    }
}

void SimulationManager::DoUpdate(float ms) {
    for (auto& p : managers) {
        auto types = p.second;
        std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*> components;
        for (int i = 0; i < (int)ComponentType::COUNT; ++i) {
            switch ((ComponentType)i) {
            case ComponentType::Spatial:
				components[ComponentType::Spatial] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&spatials);
				break;
            case ComponentType::UI:
				components[ComponentType::UI] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&uis);
				break;
            case ComponentType::SkinnedMesh:
				components[ComponentType::SkinnedMesh] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&skinnedMeshs);
				break;
            case ComponentType::Animation:
				components[ComponentType::Animation] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&animations);
				break;
            case ComponentType::PlayerControlled:
                components[ComponentType::PlayerControlled] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&playerControlled);
                break;
            case ComponentType::BoundingBox:
                components[ComponentType::BoundingBox] = reinterpret_cast<const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>(&bboxs);
                break;
            default:
                dg_assert_fail("Unhandled ComponentType");
                break;
            }
        }
		p.first->DoUpdate(components, ms);
    }
}

bool SimulationManager::HasComponent(uint64_t key, ComponentType t) {
    dg_assert_nm(key < MAX_SIM_OBJECTS);
    switch (t) {
        case ComponentType::Spatial: return spatials[key] != nullptr;
        case ComponentType::UI: return uis[key] != nullptr;
        case ComponentType::SkinnedMesh: return skinnedMeshs[key] != nullptr;
        case ComponentType::Animation: return animations[key] != nullptr;
        case ComponentType::PlayerControlled: return playerControlled[key] != nullptr;
        case ComponentType::BoundingBox: return bboxs[key] != nullptr;
        default: dg_assert_fail("type not accounted for."); break;
    }
    return false;
}

Component* SimulationManager::AddComponent(uint64_t key, ComponentType t) {
    dg_assert_nm(key < MAX_SIM_OBJECTS);

    auto checkAddComp = [](auto& cs, uint64_t key) {
        dg_assert_nm(!cs[key]);
        cs[key] = std::make_unique<std::remove_pointer_t<decltype(cs[key].get())>>();
        return cs[key].get();
    };

    switch (t) {
        case ComponentType::Spatial: return checkAddComp(spatials, key);
        case ComponentType::UI: return checkAddComp(uis, key);
        case ComponentType::SkinnedMesh: return checkAddComp(skinnedMeshs, key);
        case ComponentType::Animation: return checkAddComp(animations, key);
        case ComponentType::PlayerControlled: return checkAddComp(playerControlled, key);
        case ComponentType::BoundingBox: return checkAddComp(bboxs, key);
        default: dg_assert_fail("type not accounted for."); break;
    }
    return nullptr;
}

void SimulationManager::RemoveComponent(uint64_t key, ComponentType t) {
    dg_assert_nm(key < MAX_SIM_OBJECTS);
    switch (t) {
        case ComponentType::Spatial: return spatials[key].reset();
        case ComponentType::UI: return uis[key].reset();
        case ComponentType::SkinnedMesh: return skinnedMeshs[key].reset();
        case ComponentType::Animation: return animations[key].reset();
        case ComponentType::PlayerControlled: return playerControlled[key].reset();
        case ComponentType::BoundingBox: return bboxs[key].reset();
        default: dg_assert_fail("type not accounted for."); break;
    }
}