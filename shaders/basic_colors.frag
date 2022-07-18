#version 330 core

in vec3 i_normal;

out vec4 frag_color;

uniform vec3 color;
uniform vec3 light_dir;
uniform mat4 view_mat;

void main()
{
    float amb_light = 0.01f;
    vec4 world_light_dir = view_mat * vec4(light_dir, 0.0f);
    frag_color = vec4(color, 1.0f) * max(amb_light, dot(world_light_dir.xyz, i_normal));
    frag_color = vec4(color, 0.5f);
}
