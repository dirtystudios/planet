#pragma once 

#include "ResourceTypes.h"
namespace gfx {
struct Resource {
    virtual ~Resource() {};
    ResourceId resourceId{0};
};
}
