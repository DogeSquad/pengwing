#pragma once

#include "glm/gtc/matrix_transform.hpp"

#include <vector>

class Animator {
public:
	const unsigned int LINEAR = 0;
	const unsigned int EASE_IN_OUT = 1;
	const unsigned int EASE_IN = 2;
	const unsigned int EASE_OUT = 3;
	struct Keyframe {
		unsigned int frame;
		glm::mat4 *transform;
	};
	unsigned int currentKeyframe;
	std::vector<Keyframe*> keyframes;
	Animator(glm::mat4 *transform);
	glm::mat4 getCurrentTransform();
};