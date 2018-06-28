#pragma once

#include <vector>
#include <map>
#include "Resource.h"
#include "DGAssert.h"

namespace gfx {
class ResourceManager {
private:
    std::vector<Resource*> _resources;
public:
    ResourceId AddResource(Resource* resource);
    
    template <typename T>
    T* GetResource(ResourceId resourceId) {
        dg_assert_nm(resourceId > 0 && resourceId <= _resources.size());
        return dynamic_cast<T*>(_resources[resourceId - 1]);
    }

    template <typename T>
    bool DestroyResource(ResourceId resourceId) {
        dg_assert_nm(resourceId > 0 && resourceId <= _resources.size());
        // todo: reuse old id's
        auto res = dynamic_cast<T*>(GetResource<Resource>(resourceId));
        if (res != nullptr) {
            delete res;
            _resources[resourceId - 1] = nullptr;
        }
        return true;
    }
};
}
