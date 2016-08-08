#pragma once

#include "ResourceTypes.h"

namespace gfx {

ResourceId GenerateResourceId(ResourceType type, size_t key);
ResourceType ExtractResourceType(ResourceId id);
size_t ExtractResourceKey(ResourceId id);
}
