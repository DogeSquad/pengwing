#pragma once

#include "Object.h"

class Camera : public Object {
public:
	Camera(glm::mat4* parent, const char* name);
	void Camera::update(unsigned int frame) override;
	Animator positionAnimator;
	Animator rotationAnimator;
	Animator scaleAnimator;
};