#pragma once

#include "glm/glm.hpp"

struct Camera {
    Camera(float fov_degrees = 45.f, float aspect_ratio = 1.33333f, float znear = 0.1f, float zfar = 10000.f);

    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 look;
    glm::vec3 pos;

    float fov_degrees;
    float zfar;
    float znear;
    float aspect_ratio;

    void Translate(glm::vec3 translation);
    void MoveTo(glm::vec3 pos);
    void Translate(float x, float y, float z);
    void MoveTo(float x, float y, float z);
    void Yaw(float degrees);
    void Pitch(float degrees);
    void LookAt(glm::vec3 target);
    void LookAt(float x, float y, float z);
    
    float GetHorizontalFieldOfView();
    float GetVerticalFieldOfView();
    
    glm::mat4 BuildView();
    glm::mat4 BuildProjection();    
};