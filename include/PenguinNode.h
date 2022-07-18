#pragma once
#include "Object.h"

class PenguinNode : public Object
{
public:
	PenguinNode(glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;
	Animator positionAnimator, rotationAnimator, scaleAnimator;
};