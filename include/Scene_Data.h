#pragma once

#include "glm/glm.hpp"

#include <vector>

struct DirectionalLight
{
	glm::vec3 light_direction;
	glm::vec3 light_ambient;
	glm::vec3 light_diffuse;
	glm::vec3 light_specular;
};

struct SceneData
{
	glm::vec3 cam_pos;
	glm::mat4 view_mat;
	glm::mat4 proj_mat;
	std::vector<DirectionalLight*> directional_lights;
};