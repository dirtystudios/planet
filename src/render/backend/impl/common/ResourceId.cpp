#include "ResourceId.h"
#include "DMath.h"
#include <cassert>

static constexpr size_t kMaxResourceId = dm::pow_<size_t, 2, 56>::value - 1;
namespace gfx {
ResourceId GenerateResourceId(ResourceType type, size_t key) {
    assert(key < kMaxResourceId);
    return (static_cast<size_t>(type) << 56) | (++key & kMaxResourceId);
}
ResourceType ExtractResourceType(ResourceId id) { return static_cast<ResourceType>(id >> 56); }
size_t ExtractResourceKey(ResourceId id) { return id & kMaxResourceId; }
}
