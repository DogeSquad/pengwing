#pragma once

#include "glm/glm.hpp"
#include "Shader.h"

#include <string>
#include "glad/glad.h"

#include <iostream>
#include <vector>
// Inspired by LearnOpenGL Mesh Code

using namespace std;

class Mesh {
public:
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };
    struct Texture {
        unsigned int id;
        string type;
        string path;
    };
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    void Draw(Shader& shader);
private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};