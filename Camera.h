#pragma once

#include <glm/glm.hpp>

class Camera {
private:
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _look;
    glm::vec3 _pos;
    
    float _fov;
    float _zfar;
    float _znear;
    float _aspect_ratio;
    
    glm::mat4 _persp;
    glm::mat4 _view;

    bool _dirty_view;
    bool _dirty_perspective;    
public:
    Camera(float fov = 45.f, float aspect_ratio = 4.f/3.f, float znear = 0.1f, float zfar = 10000.f);

    void SetFieldOfView(float degrees);
    void SetFarClip(float distance);
    void SetNearClip(float distance);
    void SetAspectRatio(float aspect);

    void Translate(glm::vec3 translation);
    void MoveTo(glm::vec3 pos);
    void Yaw(float degrees);
    void Pitch(float degrees);
    void LookAt(glm::vec3 target);

    glm::vec3 GetRight();
    glm::vec3 GetUp();
    glm::vec3 GetLook();
    glm::vec3 GetPos();
    glm::mat4 GetView();
    glm::mat4 GetProjection();    

private:
    void BuildView();
    void BuildPerspective();    
   
};