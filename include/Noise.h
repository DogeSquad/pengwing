#pragma once

#include "common.hpp"
#include "glm/gtc/noise.hpp"

class Noise
{
public:
	Noise();
	unsigned int getPerlinNoiseID();
	unsigned int getWorleyNoiseID();
	void generatePerlin(glm::uvec3 size);
	void generateWorley(glm::uvec3 size, unsigned int numPoints);
private:
	unsigned int perlinNoiseID;
	unsigned int worleyNoiseID;
	float* dataPerlin;
};