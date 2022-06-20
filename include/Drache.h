#pragma once
#include "Object.h"

class Drache : public Object{
public:
	Drache(Shader shader, Model model, glm::mat4* parent, const char* name);
	void Drache::update(unsigned int frame) override;
	Animator positionAnimator;
	Animator rotationAnimator;
	Animator scaleAnimator;
};