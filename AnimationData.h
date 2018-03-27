#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct AnimationData {
    struct AnimationNode {
        struct Vec3Time {
            double time;
            glm::vec3 scale;
            Vec3Time(double t, glm::vec3 s) {
                time = t;
                scale = s;
            }
        };

        struct QuatTime {
            double time;
            glm::quat rot;
            QuatTime(double t, glm::quat q) {
                time = t;
                rot = q;
            }
        };

        std::vector<QuatTime> rotations;
        std::vector<Vec3Time> scales;
        std::vector<Vec3Time> translations;
    };
    double ticksPerSecond;
    double duration;
    std::unordered_map<std::string, AnimationNode> animationNodes;
};
