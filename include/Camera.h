#pragma once

#include "Object.h"

class Camera : public Object {
public:
	Camera(glm::mat4* parent, const char* name);
	void update(unsigned int frame) override;
	void render(SceneData& scene_data) override;
	glm::mat4 viewMatrix();

	void setupRotationAnimation(Animator* rotationAnimator);

	Animator rotationAnimator;
};