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
        dg_assert_nm(outputVector > 0);
        outputVector->clear();

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

        // for debugging
        std::sort(begin(*outputVector), end(*outputVector), [](const TerrainQuadNode* lhs, const TerrainQuadNode* rhs) {
            return (lhs->key.tx + lhs->key.ty * pow(2, lhs->key.lod) < rhs->key.tx + rhs->key.ty * pow(2, rhs->key.lod));
        });
    }

private:
    bool IsNodeInView(const Camera& camera, const TerrainQuadNode* node) { return true; }
    bool ShouldSplitNode(const Camera& camera, const TerrainQuadNode* node) {
        glm::vec3 world = node->terrain->localToWorld(node->local);
        glm::vec3 eye   = camera.pos;
        float     d     = glm::distance(world, eye);
        if (d < 1.5f * node->size) {
            return true;
        }
        return false;
        //        std::random_device rd;
        //        std::mt19937 gen(rd);
        //        std::bernoulli_distribution d(0.1);
        //        return rand() % 2 == 0 && node->key.lod < maxLod;
        //        return  node->key.lod < 2;
    }
};
