#pragma once

#include <cstdlib>
#include "ResourceTypes.h"
#include <string>

namespace graphics {
struct ShaderParamUpdate {
    ShaderParamUpdate(ShaderParamId paramId, void* data, size_t len) : paramId(paramId), len(len) {
    	this->data = malloc(len);
    	memcpy(this->data, data, len);
    };

    ~ShaderParamUpdate() {
    	free(data);
    }

    ShaderParamId paramId{0};
    void* data{nullptr};
    size_t len{0};

    std::string ToString() {
        return "ShaderParamUpdate [paramId:" + std::to_string(paramId) + ", data:" +
               std::to_string(reinterpret_cast<std::size_t>(data)) + ", len:" + std::to_string(len) + "]";
    }
};
}