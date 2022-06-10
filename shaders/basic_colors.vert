#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 i_normal;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    mat4 VM = view_mat * model_mat;
    gl_Position = proj_mat * VM * vec4(position, 1.0f);
    i_normal = normalize((transpose(inverse(VM)) * vec4(normal, 0.0)).xyz);
} 
