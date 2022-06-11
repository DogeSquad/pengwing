#pragma once
#include "Shader.h"
#include "mesh.hpp"

class Object
{
public:
	Shader shader;
	geometry object_geometry;
	bool active;
	Object(Shader shader, geometry geometry);
	void destroy();
	virtual void render(glm::mat4 view_mat, glm::mat4 proj_mat);
	virtual void update(int frame);
};