#include "Camera.h"
Camera::Camera() : Object::Object()
{
    this->position = glm::vec3(0.0f, 3.0f, 10.0f);
    this->rotation = glm::vec4(1.0001f, 0.0f, 0.0f, -0.05f * glm::half_pi<float>());
    this->rotationAnimator = Animator(this->rotation);
    this->positionAnimator = Animator(glm::vec4(this->position, 1.0f));
    setupRotationAnimation(&this->rotationAnimator);
    setupPositionAnimation(&this->positionAnimator);
}
Camera::Camera(glm::mat4* parent, const char* name) : Object::Object(parent, name)
{   
    this->position = glm::vec3(0.0f, 3.0f, 10.0f);
    this->rotation = glm::vec4(1.0001f, 0.0f, 0.0f, -0.05f * glm::half_pi<float>());
    this->rotationAnimator = Animator(this->rotation);
    this->positionAnimator = Animator(glm::vec4(this->position, 1.0f));
    setupRotationAnimation(&this->rotationAnimator);
    setupPositionAnimation(&this->positionAnimator);
}
glm::mat4 Camera::viewMatrix()
{
    return glm::inverse(model_matrix);
}
void Camera::update(unsigned int frame) 
{
    this->rotation = this->rotationAnimator.updateVector(frame);
    this->position = this->positionAnimator.updateVector(frame);
    Object::update(frame);
}

void Camera::setupRotationAnimation(Animator* rotationAnimator)
{
    rotationAnimator->AddKeyframe(120, glm::vec4(1.0001f, 0.0f, 0.0f, -0.05f * glm::half_pi<float>()), Animator::HOLD);
    rotationAnimator->AddKeyframe(180, glm::vec4(1.0001f, 0.0f, 0.0f, 0.2f * glm::half_pi<float>()), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe(300, glm::vec4(1.0001f, 0.0f, 0.0f, 0.2f * glm::half_pi<float>()), Animator::HOLD);
    rotationAnimator->AddKeyframe(360, glm::vec4(1.0001f, 0.0f, 0.0f, 0.0f), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe(1800, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), Animator::EASE_IN_OUT);
}
void Camera::setupPositionAnimation(Animator* positionAnimator)
{
    positionAnimator->AddKeyframe(480, glm::vec4(0.0f, 3.0f, 10.0f, 0.0f), Animator::HOLD);
    positionAnimator->AddKeyframe(1000, glm::vec4(0.0f, 5.0f, 30.0f, 0.0f), Animator::EASE_IN_OUT);
    positionAnimator->AddKeyframe(1800, glm::vec4(0.0f, 5.0f, 30.0f, 0.0f), Animator::HOLD);
}