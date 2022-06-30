#pragma once
#include "glad/glad.h"

#include <iostream>


enum class RenderDirection {
	A_TO_B,
	B_TO_A,
	A_TO_SCR,
	B_TO_SCR
};
class Postprocessing
{
public:
	unsigned int framebufferA;
	unsigned int framebufferB;
	Postprocessing(unsigned int WINDOW_WIDTH, unsigned int WINDOW_HEIGHT);
	void postprocess(RenderDirection rd);
	void renderQuad();
	void destroy();
private:
	const float quadVertices[24] = {
		// positions   texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int textureColorbufferA;
	unsigned int textureColorbufferB;
	unsigned int rboA;
	unsigned int rboB;
	unsigned int quadVAO;
	unsigned int quadVBO;
};