#pragma once

class Keyframe {
public:
	const unsigned int LINEAR = 0;
	const unsigned int EASE_IN_OUT = 1;
	const unsigned int EASE_IN = 2;
	const unsigned int EASE_OUT = 3;
	unsigned int frame;
};