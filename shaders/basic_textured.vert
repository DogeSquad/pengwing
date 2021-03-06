#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// declare an interface block; see 'Advanced GLSL' for what these are.
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.TexCoords = aTexCoords;  
    vs_out.Normal = aNormal;
    gl_Position = proj_mat * view_mat * model_mat * vec4(aPos, 1.0);
}