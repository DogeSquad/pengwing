#include "Noise.h"
#include "SimplexNoise.h"

#include <vector>

// Simplex Noise provided by SRombauts https://github.com/SRombauts/SimplexNoise/blob/master/src/SimplexNoise.h

Noise::Noise()
{
	glGenTextures(1, &this->perlinNoiseID);
	glGenTextures(1, &this->worleyNoiseID);
}
void Noise::generatePerlin(glm::uvec3 size)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->perlinNoiseID);
	unsigned int nPointsInSlice = size.x * size.y;
	this->perlinData = new float[nPointsInSlice * size.z];
	//auto buffer = std::make_unique<float[]>(nPointsInSlice * size.z);

	const SimplexNoise simplex(2.0f, 2.0f, 2.0f, 0.4f);
	const int octaves = static_cast<int>(8);

	for (int z = 0; z < size.z; z++)
	{
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				glm::vec3 pos = glm::vec3(2.0f * x / size.x - 1.0f, 2.0f * y / size.y - 1.0f, 2.0f * z / size.z - 1.0f);
				//float noise = glm::perlin(pos, 4.0f * glm::vec3(size));

				float noise = simplex.fractal(octaves, 2.0f * x / size.x - 1.0f, 2.0f * y / size.y - 1.0f, 2.0f * z / size.z - 1.0f);

				float amplitude = 0.5f;
				float st = 1.0f;
				for (int i = 0; i < 4; i++) {
					noise += amplitude * abs(simplex.noise(st));
					st *= 2.0f;
					amplitude *= 0.5f;
				}

				int index = (z * nPointsInSlice + y * size.x + x);
				this->perlinData[index] = noise;
			}
		}
	}
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, size.x,
		size.y, size.z, 0, GL_RED, GL_FLOAT, this->perlinData);
	glBindTexture(GL_TEXTURE_3D, 0);
}
void Noise::generateWorley(glm::uvec3 size, unsigned int numPoints)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->worleyNoiseID);
	unsigned int nPointsInSlice = size.x * size.y;
	this->worleyData = new float[nPointsInSlice * size.z];
	//auto buffer = std::make_unique<float[]>(nPointsInSlice * size.z);

	const SimplexNoise simplex(0.5f, 2.0f, 2.0f, 0.4f);
	const int octaves = static_cast<int>(8);

	std::vector<glm::vec3> points;
	for (float i = 0; i < numPoints; i++)
	{
		points.push_back(glm::vec3(size.x * (2.0f * simplex.noise(i / 64.0f) - 1.0f), 
								   size.y * (2.0f * simplex.noise(i / 26.0f) - 1.0f),
								   size.z * (2.0f * simplex.noise(i /-12.0f) - 1.0f)));
	}

	for (int z = 0; z < size.z; z++)
	{
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				glm::vec3 pos = glm::vec3(x, y, z);
				//float noise = glm::perlin(pos, 4.0f * glm::vec3(size));

				float maxDst = 0.0f;
				for (int i = 0; i < points.size(); i++)
				{
					float length = glm::length(points[i] - glm::vec3(x, y, z));
					if (length > maxDst)
					{
						maxDst = length;
					}
				}
				float noise = glm::clamp(maxDst, 0.0f, 1.0f);

				int index = (z * nPointsInSlice + y * size.x + x);
				this->worleyData[index] = noise;
			}
		}
	}
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, size.x,
		size.y, size.z, 0, GL_RED, GL_FLOAT, this->worleyData);
	glBindTexture(GL_TEXTURE_3D, 0);
}


unsigned int Noise::getPerlinNoiseID()
{
	return this->perlinNoiseID;
}
unsigned int Noise::getWorleyNoiseID()
{
	return this->worleyNoiseID;
}