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
    rotationAnimator->AddKeyframe(0, glm::vec4(0.0f, 1.0f, 0.0f, glm::pi<float>() + 0.4f), Animator::HOLD);

    rotationAnimator->AddKeyframe(950, glm::vec4(0.0f, 1.0f, 0.0f, glm::pi<float>() + 0.4f), Animator::HOLD);
    rotationAnimator->AddKeyframe(1100, glm::vec4(0.0f, 1.0f, 0.01f, glm::pi<float>() + 3.1f), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe(1210, glm::vec4(0.0f, 1.0f, 0.0f, glm::pi<float>() + 0.4f), Animator::HOLD);

    rotationAnimator->AddKeyframe(1400, glm::vec4(0.0f, 1.0f, 00.0f, 0.0f), Animator::HOLD);

    rotationAnimator->AddKeyframe(1800, glm::vec4(0.0f, 1.0f, 0.0f, glm::pi<float>() + 0.4f), Animator::HOLD);
}
void Camera::setupPositionAnimation(Animator* positionAnimator)
{
    positionAnimator->AddKeyframe(0, glm::vec4(-10.0f, 10.0f, 10.0f, 1.0f), Animator::HOLD);
    positionAnimator->AddKeyframe(400, glm::vec4(-10.0f, 10.0f, 0.0f, 1.0f), Animator::EASE_OUT);     // Penguin start Flapping
    positionAnimator->AddKeyframe(530, glm::vec4(8.0f,  9.0f, 15.0f, 1.0f), Animator::EASE_IN_OUT);
    positionAnimator->AddKeyframe(750, glm::vec4(18.4f, 5.3f, 40.0f, 1.0f), Animator::HOLD);
    positionAnimator->AddKeyframe(1210, glm::vec4(18.4f, 5.3f, 40.0f, 1.0f), Animator::HOLD);
    positionAnimator->AddKeyframe(1360, glm::vec4(18.46f, 5.3f, 40.11f, 1.0f), Animator::LINEAR);
    positionAnimator->AddKeyframe(1400, glm::vec4(-250.0f, 70.0f, 20.0f, 1.0f), Animator::HOLD);
    positionAnimator->AddKeyframe(1650, glm::vec4(-350.0f, 70.0f, 20.0f, 1.0f), Animator::EASE_OUT);

    positionAnimator->AddKeyframe(1800, glm::vec4(-4.0f, 9.0f, 15.0f, 1.0f), Animator::HOLD);
}