#include "Drache.h"


Drache::Drache(Shader shader, Model model, glm::mat4* parent, const char* name) : Object::Object(shader, model, parent, name)
{
    this->positionAnimator = Animator(glm::vec4(this->position.x, this->position.y, this->position.z, 0.0f));
    this->rotationAnimator = Animator(this->rotation);
    this->scaleAnimator = Animator(glm::vec4(this->scale.x, this->scale.y, this->scale.z, 0.0f));
    setupPositionAnimation(&this->positionAnimator);
    setupRotationAnimation(&this->rotationAnimator);
    setupScaleAnimation(&this->scaleAnimator);
}

void Drache::update(unsigned int frame)
{
    this->position = this->positionAnimator.updateVector(frame);
    this->rotation = this->rotationAnimator.updateVector(frame);
    this->scale = this->scaleAnimator.updateVector(frame);

    Object::update(frame);
}

void Drache::setupPositionAnimation(Animator* positionAnimator)
{
    positionAnimator->AddKeyframe( 120, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), Animator::HOLD);
    positionAnimator->AddKeyframe( 180, glm::vec4(0.0f, 5.0f, 0.0f, 0.0f), Animator::EASE_OUT);
    positionAnimator->AddKeyframe( 300, glm::vec4(0.0f, 5.0f, 0.0f, 0.0f), Animator::HOLD);
    positionAnimator->AddKeyframe( 360, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), Animator::EASE_OUT_BOUNCE);
    positionAnimator->AddKeyframe( 480, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), Animator::HOLD);

    positionAnimator->AddKeyframe(1800, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), Animator::HOLD);
}

void Drache::setupRotationAnimation(Animator* rotationAnimator)
{
    rotationAnimator->AddKeyframe( 120, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
    rotationAnimator->AddKeyframe( 180, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
    rotationAnimator->AddKeyframe( 240, glm::vec4(0.2f, 3.0f, 0.528f, glm::pi<float>()), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe( 300, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::EASE_IN_OUT);
    rotationAnimator->AddKeyframe( 480, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);

    rotationAnimator->AddKeyframe(1800, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
}

void Drache::setupScaleAnimation(Animator* scaleAnimator)
{
    scaleAnimator->AddKeyframe(120, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::HOLD);
    scaleAnimator->AddKeyframe(180, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::HOLD);
    scaleAnimator->AddKeyframe(240, 0.3f * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::EASE_IN_OUT);
    scaleAnimator->AddKeyframe(300, 1.0f * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::EASE_IN_OUT);
    scaleAnimator->AddKeyframe(480, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::HOLD);

    scaleAnimator->AddKeyframe(1800, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), Animator::HOLD);
}