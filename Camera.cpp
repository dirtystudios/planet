#include "camera.h"
#include <glm/gtx/rotate_vector.hpp>
#include <sstream>

Camera::Camera(float fov/* = 45.f*/, float aspect_ratio/* = 4.f / 3.f*/, float znear/* = 0.001f*/, float zfar/* = 10000.f*/)
        : _fov(fov), _zfar(zfar), _znear(znear), _aspect_ratio(aspect_ratio) {
    _pos = glm::vec3(0, 0, 0);
    _up = glm::vec3(0, 1, 0);
    _right = glm::vec3(1, 0, 0);
    _look = glm::vec3(0, 0, 1);
    BuildView();
    BuildPerspective();
}

void Camera::SetFieldOfView(float degrees) {
    _fov = degrees;
    _dirty_perspective = true;
}

void Camera::SetFarClip(float distance) {
    _zfar = distance;
    _dirty_perspective = true;
}

void Camera::SetNearClip(float distance) {
    _znear = distance;
    _dirty_perspective = true;
}

void Camera::SetAspectRatio(float aspect) {
    _aspect_ratio = aspect;
    _dirty_perspective = true;    
}

void Camera::Translate(glm::vec3 translation) {
    _pos += (_right * translation.x) + (_up * translation.y) + (_look * translation.z);
    _dirty_view = true;
}

void Camera::MoveTo(glm::vec3 pos) {
    _pos = pos;
    _dirty_view = true;
}

void Camera::Yaw(float degrees) {
    _look = glm::normalize(glm::rotate(_look, degrees, glm::vec3(0, 1, 0)));
    _right = glm::normalize(glm::rotate(_right, degrees, glm::vec3(0, 1, 0)));
    _dirty_view = true;
}

void Camera::Pitch(float degrees) {
    _look = glm::normalize(glm::rotate(_look, degrees, _right));
    _up = glm::normalize(glm::rotate(_up, degrees, _right));
    _dirty_view = true;
}

void Camera::LookAt(glm::vec3 target) {
    _look = glm::normalize(target - _pos);
    _dirty_view = true;
}

glm::vec3 Camera::GetPos() {
    return _pos;
}

glm::mat4 Camera::GetView() {
    if (_dirty_view) {
        BuildView();
    }
    return _view;
}

glm::mat4 Camera::GetProjection() {
    if (_dirty_perspective) {
        BuildPerspective();
    }
    return _persp;
}

glm::vec3 Camera::GetLook() {
    return _look;
}

glm::vec3 Camera::GetUp() {
    return _up;
}

glm::vec3 Camera::GetRight() {
    return _right;
}

void Camera::BuildView() {    
    // re-orthogonalize
    _look = glm::normalize(_look);
    _up = glm::normalize(glm::cross(_right, _look));
    _right = glm::cross(_look, _up);

    _view = glm::lookAt(_pos, _pos + _look, _up);
    _dirty_view = false;
}

void Camera::BuildPerspective() {    
    _persp = glm::perspective(_fov, _aspect_ratio, _znear, _zfar);
    _dirty_perspective = false;
}

/*
void Camera::BuildFrustum() {    
    //BuildFrustum2();
    if(_dirty_view) {
        BuildView();
    }
    static bool once = true;
    glm::vec3 near_plane_center = _pos + _look * _znear;
    glm::vec3 far_plane_center = _pos + _look * _zfar;    
    float tan_fov_x_div_two = tan(DEGREE_TO_RADIANS(_fov) / 2.f);
    float tan_fov_y_div_two = tan(DEGREE_TO_RADIANS(_fov) / _aspect_ratio / 2.f);
    float near_plane_half_width = _znear * tan_fov_x_div_two;    
    float near_plane_half_height = _znear * tan_fov_y_div_two;        
    float far_plane_half_width = _zfar * tan_fov_x_div_two;    
    float far_plane_half_height = _zfar * tan_fov_y_div_two;        

    glm::vec3 right_point_on_near = near_plane_center + (_right * near_plane_half_width);
    glm::vec3 left_point_on_near = near_plane_center - (_right * near_plane_half_width);
    glm::vec3 top_point_on_near = near_plane_center + (_up * near_plane_half_height);
    glm::vec3 bot_point_on_near = near_plane_center - (_up * near_plane_half_height);

    glm::vec3 right_point_on_far = far_plane_center + (_right * far_plane_half_width);
    glm::vec3 left_point_on_far = far_plane_center - (_right * far_plane_half_width);
    glm::vec3 top_point_on_far = far_plane_center + (_up * far_plane_half_height);
    glm::vec3 bot_point_on_far = far_plane_center - (_up * far_plane_half_height);
    
    Plane near_plane = { near_plane_center, _look };
    Plane far_plane = { far_plane_center,  -_look };
    Plane right_plane = { right_point_on_near, glm::normalize(glm::cross(_up, glm::normalize(right_point_on_near - _pos))) };
    Plane left_plane = { left_point_on_near, glm::normalize(glm::cross(glm::normalize(left_point_on_near - _pos), _up)) };
    Plane top_plane = { top_point_on_near, glm::normalize(glm::cross(glm::normalize(top_point_on_near - _pos), _right)) };
    Plane bot_plane = { bot_point_on_near, glm::normalize(glm::cross(_right, glm::normalize(bot_point_on_near - _pos))) };

    _frustum = { { near_plane, far_plane, right_plane, left_plane, top_plane, bot_plane } };
    _dirty_frustum = false;    
}
*/

