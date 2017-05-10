#pragma once

#include "MeshNode.h"
#include "MeshGeometry.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <map>

using BoneOffsetData = std::vector<std::pair<std::string, glm::mat4>>;
using MeshTreeData = std::map<uint32_t, std::vector<uint32_t>>;

class Mesh {
private:
    std::vector<MeshNode> m_nodes;
    BoneOffsetData m_boneOffsets;
    glm::mat4 m_gimt; // Global inverse matrix transform 
    MeshTreeData m_tree;
    std::vector<MeshGeometry> m_meshGeom;
public:
    Mesh(std::vector<MeshNode>&& nodes, std::vector<MeshGeometry>&& meshGeom, BoneOffsetData&& boneOffsets, glm::mat4&& gimt, MeshTreeData&& tree)
        : m_nodes(std::move(nodes)), m_meshGeom(std::move(meshGeom)), m_boneOffsets(std::move(boneOffsets)), m_gimt(std::move(gimt)), m_tree(std::move(tree)) {}

    const std::vector<MeshNode>& GetNodes() const {
        return m_nodes;
    }

    const BoneOffsetData& GetBones() const {
        return m_boneOffsets;
    }

    const MeshTreeData& GetTree() const {
        return m_tree;
    }

    const glm::mat4& GetGimt() const {
        return m_gimt;
    }

    const std::vector<MeshGeometry>& GetMeshGeometry() const {
        return m_meshGeom;
    }
};

using MeshPtr = std::shared_ptr<Mesh>;
