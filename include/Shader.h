#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader();
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec4(const std::string& name, glm::vec4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec2(const std::string& name, glm::vec2 value) const;
    int getUniformLocation(const std::string& name);
};