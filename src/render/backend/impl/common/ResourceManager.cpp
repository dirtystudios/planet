#include "ResourceManager.h"

namespace gfx {
    
ResourceId ResourceManager::AddResource(Resource* resource) {
    if(resource == nullptr) {
        return 0;
    }
    _resources.push_back(resource);
    resource->resourceId = _resources.size();
    return resource->resourceId;
}
    
bool ResourceManager::DestroyResource(ResourceId resourceId) {
    dg_assert_nm(resourceId > 0 && resourceId <= _resources.size());
    Resource* res = GetResource<Resource>(resourceId);
    _resources.erase(begin(_resources) + (resourceId - 1));
    delete res;
    return true;
}
    
}
