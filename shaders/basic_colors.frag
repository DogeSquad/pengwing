#version 330 core

in vec3 i_normal;

out vec4 frag_color;

uniform vec3 light_dir;

void main()
{
    frag_color = vec4(0.4f, 0.2f, 1.0f, 1.0f) * max(0.f, dot(light_dir, i_normal));
}
