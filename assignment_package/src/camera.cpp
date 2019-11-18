#include "camera.h"

#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp),
    polar(0.f, 0.f)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),
    polar(c.polar)
{}


void Camera::RecomputeAttributes()
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::cross(right, look);

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width/height;
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::RotateAboutUp(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), up);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), right);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}

void Camera::TranslateAlongLook(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
    ref += translation;
}
void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
    ref += translation;
}

void::Camera::RotatePolar() {
    // Normalize angles appropriately
    if (polar[0] >= 360) {
        while (polar[0] > 360) {
            polar[0] -= 360;
        }
    } else if (polar[0] < 0) {
        while (polar[0] < 0) {
            polar[0] += 360;
        }
    }
    polar[1] = glm::clamp(polar[1], -89.f, 89.f);

    glm::mat4 align = glm::rotate(glm::mat4(1.0f), glm::radians(polar[0]), glm::vec3(0.f, 1.f, 0.f)) *
                      glm::rotate(glm::mat4(1.0f), glm::radians(polar[1]), glm::vec3(1.f, 0.f, 0.f));

    look = glm::vec3(align * glm::vec4(0.f, 0.f, -1.f, 0.f));
    right = glm::vec3(align * glm::vec4(1.f, 0.f, 0.f, 0.f));
    up = glm::vec3(align * glm::vec4(world_up, 0.f));

    ref = eye + look;

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width/height;
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;

}

void Camera::TranslateAlongLookWalk(float amt) {
    glm::vec3 grounded = glm::normalize(glm::vec3(look[0], 0, look[2]));
    glm::vec3 translation = grounded * amt;
    eye += translation;
    ref += translation;
}

