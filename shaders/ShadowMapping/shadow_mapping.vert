#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 proj_mat;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 lightSpaceMatrix;

void main()
{    
    vs_out.FragPos = vec3(model_mat * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model_mat))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0f);
    gl_Position = proj_mat * view_mat * vec4(vs_out.FragPos, 1.0f);
}