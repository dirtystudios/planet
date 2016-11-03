#pragma once

#include <queue>
#include <random>
#include <string>
#include <vector>
#include "Camera.h"
#include "ConsoleCommands.h"
#include "DGAssert.h"
#include "TerrainQuadNode.h"
#include "TerrainQuadTree.h"

static int32_t  maxLod = 5;
static uint32_t seed   = 0;

class TerrainQuadNodeSelector {
private:
    TerrainQuadTreePtr _quadTree;

public:
    TerrainQuadNodeSelector(TerrainQuadTreePtr quadTree) : _quadTree(quadTree) {
        config::ConsoleCommands::getInstance().RegisterCommand("up", [&](const std::vector<std::string>& s) -> std::string {
            seed++;
            return "QuadTree up";
        });
        config::ConsoleCommands::getInstance().RegisterCommand("down", [&](const std::vector<std::string>& s) -> std::string {
            seed--;
            return "QuadTree down";
        });
    }

    void SelectQuadNodes(const Camera& camera, std::vector<const TerrainQuadNode*>* outputVector) {
        srand(seed);

        std::queue<TerrainQuadNode*> q;
        q.push(_quadTree->rootNode());

        while (!q.empty()) {
            TerrainQuadNode* node = q.front();
            q.pop();

            if (!IsNodeInView(camera, node)) {
                continue;
            }

            if (ShouldSplitNode(camera, node)) {
                node->SubdivideIfNecessary();

                for (TerrainQuadNode& child : node->children) {
                    q.push(&child);
                }
            } else {
                outputVector->push_back(node);
            }
        }
    }

private:
    bool IsNodeInView(const Camera& camera, const TerrainQuadNode* node) { return true; }
    bool ShouldSplitNode(const Camera& camera, const TerrainQuadNode* node) {
        glm::dvec3 world = node->deformedPosition();
        glm::dvec3 eye   = camera.pos;
        float      d     = glm::distance(world, eye);
        if (d < 1.5f * node->size) {
            return true;
        }
        return false;
    }
};
