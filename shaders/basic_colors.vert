#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 i_normal;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    mat4 VM = view_mat * model_mat;
    gl_Position = proj_mat * VM * vec4(position, 1.0f);
    i_normal = normalize((transpose(inverse(VM)) * vec4(normal, 0.0)).xyz);
} 
