#include "Object.h"
Object::Object(glm::mat4* parent, const char* name)
{
    this->name = name;
    this->shader = Shader();
    this->model = Model();
    this->active = true;
    this->parent = parent;
    this->position = glm::vec3(0.0f);
    this->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->scale = glm::vec3(1.0f);
}
Object::Object(Shader shader, Model model, glm::mat4* parent, const char* name)
{
    this->name = name;
    this->shader = shader;
    this->model = model;
    this->active = true;
    this->parent = parent;
    this->position = glm::vec3(0.0f);
    this->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->scale = glm::vec3(1.0f);
}
void Object::destroy() 
{

}
void Object::render(Shader* shader)
{
    this->model.Draw(*shader);
}
void Object::render(SceneData &scene_data) 
{
    if (!this->active) return;
    this->shader.use();
    this->shader.setMat4("view_mat", scene_data.view_mat);
    this->shader.setMat4("proj_mat", scene_data.proj_mat);
    this->shader.setVec3("dirLight.direction", scene_data.directional_lights[0]->light_direction);
    this->shader.setVec3("dirLight.ambient", scene_data.directional_lights[0]->light_ambient);
    this->shader.setVec3("dirLight.diffuse", scene_data.directional_lights[0]->light_diffuse);
    this->shader.setVec3("dirLight.specular", scene_data.directional_lights[0]->light_specular);

    this->model.Draw(this->shader);
}
void Object::render(glm::mat4 view_mat, glm::mat4 proj_mat)
{
    if (!this->active) return;
    this->shader.use();
    this->shader.setMat4("view_mat", view_mat);
    this->shader.setMat4("proj_mat", proj_mat);
    this->shader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    this->shader.setVec3("viewPos", glm::inverse(view_mat)[3]);

    this->model.Draw(this->shader);
}
void Object::update(unsigned int frame)
{
    this->model_matrix = glm::identity<glm::mat4>();
    this->model_matrix = glm::translate(model_matrix, this->position);
    this->model_matrix = glm::rotate(model_matrix, this->rotation.w, glm::vec3(this->rotation.x, this->rotation.y, this->rotation.z));
    this->model_matrix = glm::scale(model_matrix, this->scale);

    this->model_matrix = *this->parent * this->model_matrix;

    this->shader.use();
    this->shader.setMat4("model_mat", model_matrix);
    this->shader.setVec3("light_dir", glm::vec3(0.0f, 1.0f, 0.0f));
}