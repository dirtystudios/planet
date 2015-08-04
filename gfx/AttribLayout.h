#pragma once

#include <vector>
#include <stdint.h>
#include "VertLayout.h"

namespace graphics {
    
    struct AttribElement {
        ParamType type;
        
        const char* name;
        uint32_t location;
        uint32_t size;
        
        AttribElement(ParamType type, const char* name, uint32_t location, uint32_t size)
        : type(type), name(name), location(location), size(size) {};
    };
    
    struct AttribLayout {
        std::vector<AttribElement> elements;
        
        void Add(ParamType type, const char* name, uint32_t location, uint32_t size) {
            elements.push_back(AttribElement(type, name, location, size));
        }
    };
    
    static bool Matches(const AttribLayout &attribLayout, const VertLayout &vertLayout) {
        if(attribLayout.elements.size() != vertLayout.elements.size()) {
            return false;
        }
        
        for(uint32_t idx = 0; idx < attribLayout.elements.size(); ++idx) {
            if(attribLayout.elements[idx].type != vertLayout.elements[idx].type) {
                return false;
            }
        }
        
        return true;
    }
}

