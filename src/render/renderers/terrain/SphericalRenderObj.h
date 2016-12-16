#pragma once

#include <array>
#include "TerrainRenderObj.h"

class SphericalTerrain : public TerrainRenderObj {
private:
    struct {
        TerrainQuadTree* bottomTree;
        TerrainQuadTree* topTree;
        TerrainQuadTree* frontTree;
        TerrainQuadTree* backTree;
        TerrainQuadTree* leftTree;
        TerrainQuadTree* rightTree;
    } _cubesFaces;

    std::array<TerrainQuadTree*, 6> _quadTrees;

public:
    SphericalTerrain(double radius) {
        std::shared_ptr<SphericalDeformation> deformation = std::make_shared<SphericalDeformation>(radius);

        dm::Transform topTransform;
        topTransform.rotateDegrees(-90, glm::vec3(1, 0, 0));
        topTransform.translate(glm::vec3(0, radius / 2.f, 0));
        _cubesFaces.topTree = addQuadTree(radius, deformation, topTransform.matrix());

        dm::Transform bottomTransform;
        bottomTransform.rotateDegrees(90, glm::vec3(1, 0, 0));
        bottomTransform.translate(glm::vec3(0, -radius / 2.f, 0));
        _cubesFaces.bottomTree = addQuadTree(radius, deformation, bottomTransform.matrix());

        dm::Transform frontTransform;
        frontTransform.translate(glm::vec3(0, 0, radius / 2.f));
        _cubesFaces.frontTree = addQuadTree(radius, deformation, frontTransform.matrix());

        dm::Transform backTransform;
        backTransform.rotateDegrees(180, glm::vec3(1, 0, 0));
        backTransform.translate(glm::vec3(0, 0, -radius / 2.f));
        _cubesFaces.backTree = addQuadTree(radius, deformation, backTransform.matrix());

        dm::Transform leftTransform;
        leftTransform.rotateDegrees(-90, glm::vec3(0, 1, 0));
        leftTransform.translate(glm::vec3(-radius / 2.f, 0, 0));
        _cubesFaces.leftTree = addQuadTree(radius, deformation, leftTransform.matrix());

        dm::Transform rightTransform;
        rightTransform.rotateDegrees(90, glm::vec3(0, 1, 0));
        rightTransform.translate(glm::vec3(radius / 2.f, 0, 0));
        _cubesFaces.rightTree = addQuadTree(radius, deformation, rightTransform.matrix());

        _quadTrees[0] = _cubesFaces.topTree;
        _quadTrees[1] = _cubesFaces.bottomTree;
        _quadTrees[2] = _cubesFaces.frontTree;
        _quadTrees[3] = _cubesFaces.backTree;
        _quadTrees[4] = _cubesFaces.leftTree;
        _quadTrees[5] = _cubesFaces.rightTree;
    }

    void setTransform(const glm::mat4& transform) {
        for (TerrainQuadTree* tree : _quadTrees) {
            tree->transform() = transform * tree->transform();
        }
    }
};
