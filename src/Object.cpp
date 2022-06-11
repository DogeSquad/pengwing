#include "Object.h"

Object::Object(Shader shader, geometry object_geometry) {
	this->shader = shader;
	this->object_geometry = object_geometry;
    this->active = true;
}
void Object::destroy() {
    object_geometry.destroy();
}

void Object::render(glm::mat4 view_mat, glm::mat4 proj_mat) {
    if (!active) return;
	shader.use();
    shader.setMat4("view_mat", view_mat);
    shader.setMat4("proj_mat", proj_mat);
	object_geometry.bind();
	glDrawElements(GL_TRIANGLES, object_geometry.vertex_count, GL_UNSIGNED_INT, (void*)0);
}
void Object::update(int frame) {
    glm::mat4 model_matrix = object_geometry.transform;
    model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, sin(0.01f * frame), 0.0f));

    shader.use();
    shader.setMat4("model_mat", model_matrix);
    shader.setVec3("light_dir", glm::vec3(1.0f, 0.f, 0.f));
}