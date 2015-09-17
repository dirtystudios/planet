#pragma once

#include "ParamType.h"

namespace graphics {
    struct VertElement {
        ParamType type;
        VertElement(ParamType type) : type(type) {};
    };
    
    struct VertLayout {
        std::vector<VertElement> elements;
        size_t stride { 0 };

        VertLayout(std::vector<VertElement> vert_elements) {
            for(const VertElement &elem : vert_elements) {
                stride += SizeofParam(elem.type);
                elements.push_back(elem);
            }
        };

        VertLayout() {};

        void Add(ParamType type) {
            elements.push_back(VertElement(type));
            stride += SizeofParam(type);
        }
    };


}


