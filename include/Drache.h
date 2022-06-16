#pragma once
#include "Object.h"

class Drache : public Object{
public:
	Drache(Shader shader, std::vector<geometry> object_geometry, glm::mat4* parent, const char* name);
	void Drache::update(unsigned int frame) override;
	Animator positionAnimator;
	Animator rotationAnimator;
	Animator scaleAnimator;
};