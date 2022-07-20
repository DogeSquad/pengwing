#include "PenguinNode.h"

void setupAnimationPenguinNode(Animator* positionAnimator, Animator* rotationAnimator, Animator* scaleAnimator);

PenguinNode::PenguinNode(glm::mat4* parent, const char* name) : Object::Object(parent, name)
{
	this->positionAnimator = Animator(glm::vec4(this->position.x, this->position.y, this->position.z, 1.0f));
	this->rotationAnimator = Animator(this->rotation);
	this->scaleAnimator = Animator(glm::vec4(this->scale.x, this->scale.y, this->scale.z, 1.0f));
	setupAnimationPenguinNode(&this->positionAnimator, &this->rotationAnimator, &this->scaleAnimator);
}
void PenguinNode::update(unsigned int frame)
{
	this->position = this->positionAnimator.updateVector(frame);
	this->rotation = this->rotationAnimator.updateVector(frame);
	this->scale = this->scaleAnimator.updateVector(frame);

	Object::update(frame);
}
void setupAnimationPenguinNode(Animator* positionAnimator, Animator* rotationAnimator, Animator* scaleAnimator)
{
	// Setup Start
	positionAnimator->AddKeyframe(0, glm::vec4(20.0f, 1.2f, 60.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(0, glm::vec4(0.0f, 1.0f, 0.0f, glm::three_over_two_pi<float>()), Animator::HOLD);
	scaleAnimator->AddKeyframe(0, glm::vec4(glm::vec3(0.8f), 1.0f), Animator::HOLD);


	positionAnimator->AddKeyframe(420, glm::vec4(20.0f, 1.2f, 50.0f, 1.0f), Animator::LINEAR); 
	rotationAnimator->AddKeyframe(420, glm::vec4(0.0f, 1.0f, 0.0f, glm::three_over_two_pi<float>()), Animator::HOLD);
	
	positionAnimator->AddKeyframe(420, glm::vec4(20.0f, 1.2f, 45.0f, 1.0f), Animator::EASE_OUT); // Penguin start Flapping
	positionAnimator->AddKeyframe(460, glm::vec4(20.0f, 1.2f, 45.0f, 1.0f), Animator::HOLD);
	positionAnimator->AddKeyframe(650, glm::vec4(20.0f, 1.2f, 43.0f, 1.0f), Animator::LINEAR);

	rotationAnimator->AddKeyframe(750, glm::vec4(0.0f, 1.0f, 0.0f, glm::three_over_two_pi<float>() - 1.0f), Animator::HOLD);

	positionAnimator->AddKeyframe(1500, glm::vec4(-250.0f, 0.0f, -20.0f, 1.0f), Animator::HOLD);
	positionAnimator->AddKeyframe(1750, glm::vec4(-400.0f, 0.0f, -20.0f, 1.0f), Animator::LINEAR);


	positionAnimator->AddKeyframe(2040, glm::vec4(20.0f, 1.2f, -1.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(2040, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
	scaleAnimator->AddKeyframe(2040, glm::vec4(glm::vec3(0.8f), 1.0f), Animator::HOLD);
}