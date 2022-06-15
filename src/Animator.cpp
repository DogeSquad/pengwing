#include "Animator.h"

Animator::Animator(glm::mat4 *transform) {
	this->keyframes = std::vector<Keyframe*>();
	Keyframe start = Keyframe();
	start.frame = 0;
	start.transform = transform;
	this->keyframes.push_back(&start);
}