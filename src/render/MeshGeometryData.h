#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "VertexLayoutDesc.h"

enum class VertexComponent : uint8_t { Position = 0, Normal, Texcoord };

class MeshGeometryData {
public:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<uint32_t>  indices;

    bool hasComponent(VertexComponent component) const {
        switch (component) {
            case VertexComponent::Position:
                return positions.size() > 0;
            case VertexComponent::Normal:
                return normals.size() > 0;
            case VertexComponent::Texcoord:
                return texcoords.size() > 0;
            default:
                assert(false);
        }
        return false;
    }

    gfx::VertexLayoutDesc vertexLayout() const {
        gfx::VertexLayoutDesc vertexLayout;

        if (hasComponent(VertexComponent::Position)) {
            vertexLayout.elements.push_back(
                gfx::VertexLayoutElement(gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float));
        }
        if (hasComponent(VertexComponent::Normal)) {
            vertexLayout.elements.push_back(gfx::VertexLayoutElement(gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Normal, gfx::VertexAttributeStorage::Float));
        }
        if (hasComponent(VertexComponent::Texcoord)) {
            vertexLayout.elements.push_back(
                gfx::VertexLayoutElement(gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float));
        }

        return vertexLayout;
    }

    bool hasIndices() const { return indices.size() > 0; }

    uint32_t vertexCount() const {
        // Todo: actually do this correctly.
        return positions.size();
    }

    uint32_t indexCount() const { return indices.size(); }

    // i cannot describe how much i hate this.
    void interleave(uint8_t* outputData) const {
        size_t offset = 0;
        for (uint32_t idx = 0; idx < vertexCount(); ++idx) {
            if (hasComponent(VertexComponent::Position)) {
                memcpy(outputData + offset, &positions[idx], sizeof(glm::vec3));
                offset += sizeof(glm::vec3);
            }
            if (hasComponent(VertexComponent::Normal)) {
                memcpy(outputData + offset, &normals[idx], sizeof(glm::vec3));
                offset += sizeof(glm::vec3);
            }
            if (hasComponent(VertexComponent::Texcoord)) {
                memcpy(outputData + offset, &texcoords[idx], sizeof(glm::vec2));
                offset += sizeof(glm::vec2);
            }
        }
    }
};
