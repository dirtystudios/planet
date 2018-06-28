#include "ResourceManager.h"

#include <numeric>

namespace gfx {
    
ResourceId ResourceManager::AddResource(Resource* resource) {
    if (resource == nullptr) {
        return 0;
    }
    _resources.push_back(resource);
    dg_assert_nm(_resources.size() != std::numeric_limits<size_t>::max());
    resource->resourceId = _resources.size();
    return resource->resourceId;
}
}
