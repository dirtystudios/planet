#pragma once 

#include "MeshNode.h"
#include "BoundingBox.h"
#include <vector>
#include <string>

namespace meshImport {
    struct MeshData {
        std::vector<MeshNode> nodes;
        std::vector<MeshGeometryData> geomData;
        std::vector<std::pair<std::string, glm::mat4>> boneInfo;
        glm::mat4 gimt;
        std::map<uint32_t, std::vector<uint32_t>> tree;
        std::vector<dm::BoundingBox> bboxs;
    };
    MeshData LoadMeshDataFromFile(const std::string& fpath);
}