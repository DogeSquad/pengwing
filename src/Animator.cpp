#include "Animator.h"

// Forward Declaration
// Easing function
float easeInCubic(float x);
float easeOutCubic(float x);
float easeInOutCubic(float x);
float easeOutBounce(float x);

Animator::Animator()
{
	this->keyframes = std::vector<Keyframe*>();

	AddKeyframe(0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), HOLD);
}
Animator::Animator(glm::vec4 vector) 
{
	this->keyframes = std::vector<Keyframe*>();

	AddKeyframe(0, vector, HOLD);
}

void Animator::AddKeyframe(unsigned int frame, glm::vec4 vector, unsigned int interpolation)
{	
	for (unsigned int i = 0; i < keyframes.size(); i++)
	{
		if (keyframes[i]->frame == frame) return;
		if (keyframes[i]->frame > frame) break;
	}
	Keyframe* newKeyframe = new Keyframe();
	newKeyframe->frame = frame;
	newKeyframe->vector = vector;
	newKeyframe->interpolation = interpolation;
	this->keyframes.push_back(newKeyframe);
}

glm::vec4 Animator::updateVector(unsigned int frame)
{
	// Optimierungsvorschlag: Binary Search
	unsigned int next_keyframe_num = 1;
	for (unsigned int i = 1; i < keyframes.size(); i++)
	{
		if (keyframes[i]->frame > frame) 
		{
			next_keyframe_num = i;
			break;
		}
	}

	Keyframe nextKeyframe = *keyframes[next_keyframe_num];
	Keyframe currentKeyframe = *keyframes[next_keyframe_num - 1];

	switch (nextKeyframe.interpolation) {
	case HOLD: 
		{
			return currentKeyframe.vector;
			break;
		}
	case LINEAR:
		{
			float t = ((float)frame - currentKeyframe.frame) / (nextKeyframe.frame - currentKeyframe.frame);
			glm::vec4 diff = nextKeyframe.vector - currentKeyframe.vector;
			return currentKeyframe.vector + t * diff;
			break;
		}
	case EASE_IN:
		{
			float t = ((float)frame - currentKeyframe.frame) / (nextKeyframe.frame - currentKeyframe.frame);
			glm::vec4 diff = nextKeyframe.vector - currentKeyframe.vector;
			t = easeInCubic(t);
			return currentKeyframe.vector + t * diff;
			break;
		}
	case EASE_OUT:
		{
			float t = ((float)frame - currentKeyframe.frame) / (nextKeyframe.frame - currentKeyframe.frame);
			glm::vec4 diff = nextKeyframe.vector - currentKeyframe.vector;
			t = easeOutCubic(t);
			return currentKeyframe.vector + t * diff;
			break;
		}
	case EASE_IN_OUT:
		{
			float t = ((float)frame - currentKeyframe.frame) / (nextKeyframe.frame - currentKeyframe.frame);
			glm::vec4 diff = nextKeyframe.vector - currentKeyframe.vector;
			t = easeInOutCubic(t);
			return currentKeyframe.vector + t * diff;
			break;
		}
	case EASE_OUT_BOUNCE:
		{
			float t = ((float)frame - currentKeyframe.frame) / (nextKeyframe.frame - currentKeyframe.frame);
			glm::vec4 diff = nextKeyframe.vector - currentKeyframe.vector;
			t = easeOutBounce(t);
			return currentKeyframe.vector + t * diff;
			break;
		}
	default:
		{
			return currentKeyframe.vector;
			break;
		}
	}
}

// Easing Functions from https://easings.net/de
float easeInCubic(float x)
{
	return x * x * x;
}
float easeOutCubic(float x)
{
	return 1 - pow(1 - x, 3);
}
float easeInOutCubic(float x)
{
	return x < 0.5 ? (4 * x * x * x) : (1 - pow(-2 * x + 2, 3) / 2);
}
float easeOutBounce(float x)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (x < 1.0f / d1) {
		return n1 * x * x;
	}
	else if (x < 2.0f / d1) {
		return n1 * (x -= 1.5f / d1) * x + 0.75f;
	}
	else if (x < 2.5f / d1) {
		return n1 * (x -= 2.25f / d1) * x + 0.9375f;
	}
	else {
		return n1 * (x -= 2.625f / d1) * x + 0.984375f;
	}
}