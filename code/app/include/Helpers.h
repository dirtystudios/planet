#ifndef __helpers_h__
#define __helpers_h__

#include "Log.h"

const glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
const glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
const glm::vec3 Z_AXIS = glm::vec3(0, 0, 1);
const float PI = 3.1415926f;

static void Orthogonalize(glm::vec3* v1, glm::vec3* v2, glm::vec3* v3) {
    *v1 = glm::normalize(*v1);
    *v2 = glm::normalize(glm::cross(*v3, *v1));
    *v3 = glm::cross(*v1, *v2);
}

static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    return os << v.x << ", " << v.y << ", " << v.z;
}

static inline std::string ToString(const glm::vec3& v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}

static inline float ToRadians(float degrees) {
    return degrees * PI / 180.f;
}

template <class T>
static inline void HashCombine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

static inline glm::vec3 GetColor(uint32_t index) {
    if(index > 64) {
        return glm::vec3(1, 1, 1);
    }

    static const uint32_t colors[64] = {
        0x000000, 0xFFFF00, 0x1CE6FF, 0xFF34FF, 0xFF4A46, 0x008941, 0x006FA6, 0xA30059,
        0xFFDBE5, 0x7A4900, 0x0000A6, 0x63FFAC, 0xB79762, 0x004D43, 0x8FB0FF, 0x997D87,
        0x5A0007, 0x809693, 0xFEFFE6, 0x1B4400, 0x4FC601, 0x3B5DFF, 0x4A3B53, 0xFF2F80,
        0x61615A, 0xBA0900, 0x6B7900, 0x00C2A0, 0xFFAA92, 0xFF90C9, 0xB903AA, 0xD16100,
        0xDDEFFF, 0x000035, 0x7B4F4B, 0xA1C299, 0x300018, 0x0AA6D8, 0x013349, 0x00846F,
        0x372101, 0xFFB500, 0xC2FFED, 0xA079BF, 0xCC0744, 0xC0B9B2, 0xC2FF99, 0x001E09,
        0x00489C, 0x6F0062, 0x0CBD66, 0xEEC3FF, 0x456D75, 0xB77B68, 0x7A87A1, 0x788D66,
        0x885578, 0xFAD09F, 0xFF8A9A, 0xD157A0, 0xBEC459, 0x456648, 0x0086ED, 0x886F4C,
    };
    
    uint32_t mask = ~( ((1 << 16) - 1) << 8);
    float r = (colors[index] >> 16 & mask) / 255.f;
    float g = (colors[index] >> 8 & mask) / 255.f;
    float b = (colors[index] & mask) / 255.f;
    return glm::vec3(r, g, b);
}

static std::string ReadFileContents(const std::string& fpath) {
   std::ifstream fin(fpath);

    if(fin.fail()) {
        LOG_E("Failed to open file '%s'\n", fpath.c_str());
        assert(false);
    }

    std::string ss((std::istreambuf_iterator<char>(fin)),
                    std::istreambuf_iterator<char>());

    return ss;
}




#endif