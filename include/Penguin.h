#pragma once
#include "Object.h"

class Penguin : public Object
{
public:
	Penguin(Shader shader, Model model, glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;
	Animator positionAnimator, rotationAnimator, scaleAnimator;
};