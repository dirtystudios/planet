#pragma once

#include "ComponentType.h"
#include "IdObj.h"
#include "SimulationManager.h"
#include <vector>

class SimObj : public IdObj {
private:
    SimulationManager* manager = nullptr;

public:
    SimObj(uint64_t key, SimulationManager* m)
        : IdObj(key)
        , manager(m){};
    ~SimObj() {}

    bool HasComponents(const std::vector<ComponentType>& components) {
        for (const ComponentType& type : components) {
            if (!manager->HasComponent(_id, type)) {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    T* GetComponent() {
        return manager->GetComponent(_id, T::type());
    }

    template <typename T>
    T* AddComponent() {
        return (T*)manager->AddComponent(_id, T::type());
    }

    template <typename T>
    void RemoveComponent() {
        manager->RemoveComponent(_id, T::type());
    }
};
