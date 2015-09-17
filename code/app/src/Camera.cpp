
#include "Camera.h"
#include "Helpers.h"

Camera::Camera(float fov_degrees/* = 45.f*/, float aspect_ratio/* = 4.f / 3.f*/, float znear/* = 0.001f*/, float zfar/* = 10000.f*/)
        : fov_degrees(fov_degrees), zfar(zfar), znear(znear), aspect_ratio(aspect_ratio) {
    pos = glm::vec3(0, 0, 0);
    up = glm::vec3(0, 1, 0);
    right = glm::vec3(1, 0, 0);
    look = glm::vec3(0, 0, 1);    
}

void Camera::Translate(glm::vec3 translation) {
    pos += (right * translation.x) + (up * translation.y) + (look * translation.z);
}
void Camera::MoveTo(glm::vec3 new_pos) {
    pos = new_pos;
}

void Camera::Translate(float x, float y, float z) {
    Translate(glm::vec3(x, y, z));
}

void Camera::MoveTo(float x, float y, float z) {
    MoveTo(glm::vec3(x, y, z));
}

void Camera::Yaw(float degrees) {
    look = glm::normalize(glm::rotate(look, degrees, Y_AXIS));
    right = glm::normalize(glm::rotate(right, degrees, Y_AXIS));    
}
void Camera::Pitch(float degrees) {
    look = glm::normalize(glm::rotate(look, degrees, right));
    up = glm::normalize(glm::rotate(up, degrees, right));
}
void Camera::LookAt(glm::vec3 target) {
    look = glm::normalize(target - pos);
}
void Camera::LookAt(float x, float y, float z) {
    LookAt(glm::vec3(x, y, z));
}

glm::mat4 Camera::BuildView() {
    Orthogonalize(&look, &up, &right);
    return glm::lookAt(pos, pos + look, up);
}

glm::mat4 Camera::BuildProjection() {
    return glm::perspective(fov_degrees, aspect_ratio, znear, zfar);
}
