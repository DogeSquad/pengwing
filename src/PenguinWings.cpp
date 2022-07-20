#include "PenguinWings.h"

void setupAnimationPenguinWings(Animator* positionAnimator, Animator* rotationAnimator, Animator* scaleAnimator);

PenguinWings::PenguinWings(glm::mat4* parent, const char* name) : Object::Object(parent, name)
{
	this->positionAnimator = Animator(glm::vec4(this->position.x, this->position.y, this->position.z, 1.0f));
	this->rotationAnimator = Animator(this->rotation);
	this->scaleAnimator = Animator(glm::vec4(this->scale.x, this->scale.y, this->scale.z, 1.0f));
	setupAnimationPenguinWings(&this->positionAnimator, &this->rotationAnimator, &this->scaleAnimator);
}
void PenguinWings::update(unsigned int frame)
{
	this->position = this->positionAnimator.updateVector(frame);
	this->rotation = this->rotationAnimator.updateVector(frame);
	this->scale = this->scaleAnimator.updateVector(frame);

	Object::update(frame);
}
void setupAnimationPenguinWings(Animator* positionAnimator, Animator* rotationAnimator, Animator* scaleAnimator)
{
	// Setup Start
	positionAnimator->AddKeyframe(0, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(0, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
	scaleAnimator->AddKeyframe(0, glm::vec4(1.0f), Animator::HOLD);

	int frameStart = 460;
	int frameEnd = 650;
	// Wing Flapping
	//for (int i = 0; i < 30; i+=2)
	//{
	//	scaleAnimator->AddKeyframe(frameStart + i * 5, glm::vec4(1.0f, -0.1f, 1.0f, 1.0f), Animator::LINEAR);
	//	scaleAnimator->AddKeyframe(frameStart + i * 5 + 5, glm::vec4(1.0f, 0.1f, 1.0f, 1.0f), Animator::LINEAR);
	//}
	for (int i = 0; i < 30; i+=2)
	{
		rotationAnimator->AddKeyframe(frameStart + i * 5    , glm::vec4(1.0f, 0.0f, 0.0f, -0.1f), Animator::LINEAR);
		rotationAnimator->AddKeyframe(frameStart + i * 5 + 5, glm::vec4(1.0f, 0.0f, 0.0f, 0.1f), Animator::LINEAR);
	}



	positionAnimator->AddKeyframe(2040, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(2040, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
	scaleAnimator->AddKeyframe(2040, glm::vec4(1.0f), Animator::HOLD);
}
