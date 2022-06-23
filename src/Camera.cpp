#include "Camera.h"

Camera::Camera(glm::mat4* parent, const char* name) : Object::Object(parent, name)
{   
    this->position = glm::vec3(0.0f, 0.0f, 5.0f);
    this->rotationAnimator = Animator(this->rotation);
    setupRotationAnimation(&this->rotationAnimator);
}
glm::mat4 Camera::viewMatrix()
{
    return glm::inverse(model_matrix);
}
void Camera::render(SceneData& scene_data)
{

}
void Camera::update(unsigned int frame) 
{
    this->rotation = this->rotationAnimator.updateVector(frame);
    Object::update(frame);
}

void Camera::setupRotationAnimation(Animator* rotationAnimator)
{
    rotationAnimator->AddKeyframe(0, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
    rotationAnimator->AddKeyframe(60, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
    rotationAnimator->AddKeyframe(180, glm::vec4(0.0f, 1.0f, 0.0f, -0.5 * glm::half_pi<float>()), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe(300, glm::vec4(0.0f, 1.0f, 0.0f, 0.5 * glm::half_pi<float>()), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe(420, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::EASE_IN_OUT);
}