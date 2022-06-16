#include "Object.h"

Object::Object(Shader shader, std::vector<geometry> object_geometry, glm::mat4* parent, const char* name)
{
    this->name = name;
    this->shader = shader;
    this->object_geometry = object_geometry;
    this->active = true;
    this->parent = parent;
    this->position = glm::vec3(0.0f);
    this->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->scale = glm::vec3(1.0f);
}
void Object::destroy() 
{
    for (int i = 0; i < this->object_geometry.size(); i++)
    {
        this->object_geometry[i].destroy();
    }
}
void Object::render(glm::mat4 view_mat, glm::mat4 proj_mat) 
{
    if (!this->active) return;
    this->shader.use();
    this->shader.setMat4("view_mat", view_mat);
    this->shader.setMat4("proj_mat", proj_mat);

    for (int i = 0; i < this->object_geometry.size(); i++)
    {
        this->object_geometry[i].bind();
	    glDrawElements(GL_TRIANGLES, this->object_geometry[i].vertex_count, GL_UNSIGNED_INT, (void*)0);
    }
}
void Object::update(unsigned int frame)
{
    if (!this->active) return;
    this->model_matrix = glm::identity<glm::mat4>();
    this->model_matrix = glm::translate(model_matrix, this->position);
    this->model_matrix = glm::rotate(model_matrix, this->rotation.w, glm::vec3(this->rotation.x, this->rotation.y, this->rotation.z));
    this->model_matrix = glm::scale(model_matrix, this->scale);

    this->model_matrix = *parent * this->model_matrix;

    this->shader.use();
    this->shader.setMat4("model_mat", model_matrix);
    this->shader.setVec3("light_dir", glm::vec3(0.0f, 1.0f, 0.0f));
}