#pragma once
#include "Object.h"

class PenguinWings : public Object
{
public:
	PenguinWings(glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;
	Animator positionAnimator, rotationAnimator, scaleAnimator;
};