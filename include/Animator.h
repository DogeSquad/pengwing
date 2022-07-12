#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <random>
#include <vector>

class Animator {
public:
	static const unsigned int HOLD = 0;
	static const unsigned int LINEAR = 1;
	static const unsigned int EASE_IN_OUT = 2;
	static const unsigned int EASE_IN = 3;
	static const unsigned int EASE_OUT = 4;
	static const unsigned int EASE_OUT_BOUNCE = 5;
	static const unsigned int EASE_OUT_ELASTIC = 6;
	static const unsigned int EASE_OUT_SHAKE = 7;
	static const unsigned int EASE_IN_OUT_SINE = 8;
	static const unsigned int EASE_IN_SINE = 9;
	static const unsigned int EASE_OUT_SINE = 10;
	struct Keyframe {
		unsigned int frame;
		glm::vec4 vector;
		unsigned int interpolation;
	};
	std::vector<Keyframe*> keyframes;
	Animator();
	Animator(glm::vec4 vector);
	void AddKeyframe(unsigned int frame, glm::vec4 vector, unsigned int interpolation);
	glm::vec4 updateVector(unsigned int frame);
};