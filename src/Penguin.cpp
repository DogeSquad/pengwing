#include "Penguin.h"

void setupAnimation(Animator* positionAnimator, Animator* rotationAnimator, Animator* scaleAnimator);
void waddle(unsigned int frameStart, unsigned int frameEnd, float height, glm::vec4 initPos, Animator* positionAnimator, Animator* rotationAnimator);

Penguin::Penguin(Shader shader, Model model, glm::mat4* parent, const char* name) : Object::Object(shader, model, parent, name)
{
	this->positionAnimator = Animator(glm::vec4(this->position.x, this->position.y, this->position.z, 1.0f));
	this->rotationAnimator = Animator(this->rotation);
	this->scaleAnimator = Animator(glm::vec4(this->scale.x, this->scale.y, this->scale.z, 1.0f));
	setupAnimation(&this->positionAnimator, &this->rotationAnimator, &this->scaleAnimator);
}
void Penguin::update(unsigned int frame)
{
	this->position = this->positionAnimator.updateVector(frame);
	this->rotation = this->rotationAnimator.updateVector(frame);
	this->scale = this->scaleAnimator.updateVector(frame);

	Object::update(frame);
}
void setupAnimation(Animator *positionAnimator, Animator *rotationAnimator, Animator *scaleAnimator)
{
	// Setup Start
	positionAnimator->AddKeyframe(0, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), Animator::LINEAR);
	rotationAnimator->AddKeyframe(0, glm::vec4(0.0f, 0.0f, 1.0f, -0.2f), Animator::HOLD);
	scaleAnimator->AddKeyframe(0, glm::vec4(glm::vec3(0.8f), 1.0f), Animator::HOLD);


	waddle(  0,  60, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle( 60, 120, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle(120, 180, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle(180, 240, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle(240, 300, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle(300, 360, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	waddle(360, 420, 0.5f, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), positionAnimator, rotationAnimator);
	positionAnimator->AddKeyframe(420, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), Animator::LINEAR);
	rotationAnimator->AddKeyframe(420, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), Animator::HOLD);
	
	positionAnimator->AddKeyframe(460, glm::vec4(0.2f, 0.0f, 0.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(460, glm::vec4(0.0f, 0.0f, 1.0f, -0.4f), Animator::EASE_OUT);

	positionAnimator->AddKeyframe(500, glm::vec4(0.0f, 5.0f, 0.0f, 1.0f), Animator::EASE_OUT);
	rotationAnimator->AddKeyframe(500, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), Animator::EASE_OUT);

	positionAnimator->AddKeyframe(650, glm::vec4(0.0f, 4.6f, 0.0f, 1.0f), Animator::LINEAR);
	rotationAnimator->AddKeyframe(650, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), Animator::HOLD);
	
	positionAnimator->AddKeyframe(690, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), Animator::EASE_IN);
	rotationAnimator->AddKeyframe(690, glm::vec4(0.0f, 0.01f, 1.001f, glm::half_pi<float>()), Animator::LINEAR);

	positionAnimator->AddKeyframe(750, glm::vec4(0.0f, -5.0f, 0.0f, 1.0f), Animator::HOLD);

	positionAnimator->AddKeyframe(800, glm::vec4(0.0f, -3.0f, 0.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(800, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), Animator::HOLD);

	positionAnimator->AddKeyframe(830, glm::vec4(0.0f, 1.5f, 0.0f, 1.0f), Animator::EASE_OUT);
	positionAnimator->AddKeyframe(870, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), Animator::EASE_OUT_BOUNCE);


	positionAnimator->AddKeyframe(1400, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), Animator::HOLD);

	int frameStart = 1520;
	int frameEnd = 1700;
	int duration = frameEnd - frameStart;
	float heightMin = 30.0f;
	float heigthMax = 70.0f;
	float heightDifference = heigthMax - heightMin;
	int stepSize = 5;
	for (int i = 0; i < duration; i+= stepSize)
	{
		positionAnimator->AddKeyframe(frameStart + i, glm::vec4(0.0f, heightMin + heightDifference * glm::pow((1.0f - (2.0f * (i / stepSize / duration) - 1.0f)), 2), 0.0f, 1.0f), Animator::LINEAR);
	}



	positionAnimator->AddKeyframe(1800, glm::vec4(0.0f, 0.2f, 0.0f, 1.0f), Animator::HOLD);
	rotationAnimator->AddKeyframe(1800, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), Animator::HOLD);
	scaleAnimator->AddKeyframe(1800, glm::vec4(glm::vec3(0.8f), 1.0f), Animator::HOLD);
}

void waddle(unsigned int frameStart, unsigned int frameEnd, float height, glm::vec4 initPos, Animator *positionAnimator, Animator *rotationAnimator)
{
	int duration = frameEnd - frameStart;
	int quarter_duration = duration / 4;
	if (duration <= 0) return;

	positionAnimator->AddKeyframe(frameStart + quarter_duration, initPos + glm::vec4(0.0f, height, 0.0f, 1.0f), Animator::EASE_OUT_SINE);
	positionAnimator->AddKeyframe(frameStart + 2 * quarter_duration, initPos + glm::vec4(0.0f, 0.0f, -0.2f, 1.0f), Animator::EASE_IN_SINE);
	positionAnimator->AddKeyframe(frameStart + 3 * quarter_duration, initPos + glm::vec4(0.0f, height, 0.0f, 1.0f), Animator::EASE_OUT_SINE);
	positionAnimator->AddKeyframe(frameEnd, initPos + glm::vec4(0.0f, 0.0f, 0.2f, 1.0f), Animator::EASE_IN_SINE);

	rotationAnimator->AddKeyframe(frameStart + quarter_duration, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), Animator::EASE_IN);
	rotationAnimator->AddKeyframe(frameStart + 2 * quarter_duration, glm::vec4(1.0f, 0.0f, 0.0f, 0.1f), Animator::EASE_OUT);
	rotationAnimator->AddKeyframe(frameStart + 3 * quarter_duration, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), Animator::EASE_IN);
	rotationAnimator->AddKeyframe(frameEnd, glm::vec4(1.0f, 0.0f, 0.0f, -0.1f), Animator::EASE_OUT);
}