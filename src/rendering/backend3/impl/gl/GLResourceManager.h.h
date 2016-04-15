#pragma once

#include "GLStructs.h"

class GLResourceManager {
public:
    template <typename T> T* Create() {}
    T* Find(ResourceId id);
    void Delete(ResourceId id) void Delete(T* resource)
};