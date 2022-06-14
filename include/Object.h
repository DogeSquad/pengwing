#pragma once
#include "Shader.h"
#include "mesh.hpp"

class Object
{
public:
	Shader shader;
	geometry object_geometry;
	bool active;
	const char* name;
	Object(Shader shader, geometry geometry, const char* name);
	void destroy();
	virtual void render(glm::mat4 view_mat, glm::mat4 proj_mat);
	virtual void update(int frame);
};