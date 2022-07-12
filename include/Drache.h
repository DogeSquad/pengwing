#pragma once
#include "Object.h"

class Drache : public Object
{
public:
	Drache(Shader shader, Model model, glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;

	void setupPositionAnimation(Animator* positionAnimator);
	void setupRotationAnimation(Animator* rotationAnimator);
	void setupScaleAnimation(Animator* scaleAnimator);

	Animator positionAnimator;
	Animator rotationAnimator;
	Animator scaleAnimator;
};