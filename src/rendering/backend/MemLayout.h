#ifndef __mem_layout_h__
#define __mem_layout_h__

namespace graphics {

   struct StructureElement {
        ParamType type;
        uint32_t count;
        
        StructureElement(ParamType type, uint32_t count) : type(type), count(count) {};
    };
    
    struct MemoryLayout {
        std::vector<StructureElement> elements;
        size_t stride { 0 };
        
        MemoryLayout() {};
        MemoryLayout(std::vector<StructureElement> elements) {
            for(const StructureElement &elem : elements) {
                stride += SizeofParam(elem.type) * elem.count;
                elements.push_back(elem);
            }
        };

        void Add(ParamType type, uint32_t count) {
            elements.push_back(StructureElement(type, count));
            stride += SizeofParam(type) * count;
        }
    };
}

#endif