#pragma once
#include "Shader.h"
#include "Animator.h"
#include "mesh.hpp"
#include "glm/glm.hpp"

class Object
{
public:
	Shader shader;
	geometry object_geometry;
	glm::mat4* parent;
	glm::mat4 model_matrix;
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec4 rotation;
	bool active;
	const char* name;
	Object(Shader shader, geometry geometry, glm::mat4* parent, const char* name);
	void destroy();
	virtual void render(glm::mat4 view_mat, glm::mat4 proj_mat);
	virtual void update(unsigned int frame);
};