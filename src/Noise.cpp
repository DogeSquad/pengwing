#include "Noise.h"

Noise::Noise()
{
	this->dataPerlin = nullptr;
	glGenTextures(1, &this->perlinNoiseID);
	glGenTextures(1, &this->worleyNoiseID);
}
void Noise::generatePerlin(glm::uvec3 size)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, this->perlinNoiseID);
	unsigned nPointsInSlice = size.x * size.y;
	this->dataPerlin = new float[nPointsInSlice * size.z];

	for (int z = 0; z < size.z; z++)
	{
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				float noise = glm::perlin(glm::vec3(x, y, z));
				this->dataPerlin[(z * nPointsInSlice + y * size.x + x)] = noise;
			}
		}
	}
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, size.x,
		size.y, size.z, 0, GL_R, GL_FLOAT, this->dataPerlin);
	glBindTexture(GL_TEXTURE_3D, 0);
}
void Noise::generateWorley(glm::uvec3 size, unsigned int numPoints)
{
}


unsigned int Noise::getPerlinNoiseID()
{
	return this->perlinNoiseID;
}
unsigned int Noise::getWorleyNoiseID()
{
	return this->worleyNoiseID;
}