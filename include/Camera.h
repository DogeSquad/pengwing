#pragma once

#include "Object.h"

class Camera : public Object {
public:
	Camera();
	Camera(glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;
	glm::mat4 viewMatrix();

	void setupRotationAnimation(Animator* rotationAnimator);
	void setupPositionAnimation(Animator* positionAnimator);

	Animator rotationAnimator;
	Animator positionAnimator;
};