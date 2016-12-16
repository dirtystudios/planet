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

static int32_t maxLod = 5;
static uint32_t seed = 0;

class TerrainQuadNodeSelector {
public:
    void SelectQuadNodes(const FrameView* view, TerrainQuadTree* tree, std::vector<const TerrainQuadNode*>* outputVector) {

        std::queue<TerrainQuadNode*> q;
        q.push(tree->rootNode());

        while (!q.empty()) {
            TerrainQuadNode* node = q.front();
            q.pop();

            if (!IsNodeInView(view, node)) {
                continue;
            }

            if (ShouldSplitNode(view, node)) {
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
    bool IsNodeInView(const FrameView* view, const TerrainQuadNode* node) {
        dm::Rect3Dd rect = node->worldRect();
        dm::BoundingBox box(rect.bl(), rect.tr());
        return view->frustum.IsBoxInFrustum(box);
    }
    bool ShouldSplitNode(const FrameView* view, const TerrainQuadNode* node) {
        if (node->key.lod >= 15)
            return false;
        dm::Rect3Dd rect = node->worldRect();
        dm::BoundingBox box(rect.bl(), rect.tr());
        float d = box.distance(view->eyePos);
        if (d < 1.5f * node->size) {
            return true;
        }
        return false;
    }
};
