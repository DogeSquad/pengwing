#pragma once

#include "common.hpp"

struct camera_orbital {
    camera_orbital();
    camera_orbital(GLFWwindow* window);

    virtual ~camera_orbital();

    glm::mat4
    view_matrix() const;

    glm::vec3
    position() const;
};
