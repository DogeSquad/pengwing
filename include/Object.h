#pragma once
#include "Shader.h"
#include "Animator.h"
#include "Model.h"
#include "Scene_Data.h"
#include "glm/glm.hpp"

class Object
{
public:
	Shader shader;
	Model model;
	glm::mat4* parent;
	glm::mat4 model_matrix;
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec4 rotation;
	bool active;
	const char* name;
	Object(glm::mat4* parent, const char* name);
	Object(Shader shader, Model model, glm::mat4* parent, const char* name);
	void destroy();
	virtual void render(Shader* shader);
	virtual void render(glm::mat4 view_mat, glm::mat4 proj_mat);
	virtual void update(unsigned int frame);
};