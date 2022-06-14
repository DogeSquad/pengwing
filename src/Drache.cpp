#include "Drache.h"

Drache::Drache(Shader shader, geometry object_geometry, const char* name) : Object(shader, object_geometry, name) {}

void Drache::update(int frame){
    glm::mat4 model_matrix = this->object_geometry.transform;
    model_matrix = glm::rotate(model_matrix, 0.05f * frame, glm::vec3(0.0f, 1.0f, 0.0f));

    this->shader.use();
    this->shader.setMat4("model_mat", model_matrix);
    this->shader.setVec3("light_dir", glm::vec3(0.0f, 1.0f, 0.0f));
}