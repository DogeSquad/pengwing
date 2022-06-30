#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform float near;
uniform float far;
uniform vec2 uRes;

uniform float fov;

out vec3 viewOrigin;
out vec3 viewDir;

void main()
{
    gl_Position = vec4(aPos.xy, 0.0f, 1.0f);
    TexCoords = aTexCoords;

    mat4 invprojview = inverse(proj_mat) * inverse(view_mat);
    viewOrigin = (invprojview * vec4(aPos.xy, -1.0f, 1.0f) * near).xyz;
    viewDir = (invprojview * vec4(aPos.xy * (far - near), far + near, far - near)).xyz;
}  