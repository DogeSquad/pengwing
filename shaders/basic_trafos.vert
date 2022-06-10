#version 330 core
layout (location = 0) in vec3 position;

uniform float uTime;

// Translation
mat4 mov = mat4( 1. , 0. , 0. , 0.,
                 0. , 1. , 0. , 0.,
                 0. , 0. , 1. , 0.,
                 0. , 0. , 0. , 1.);

// Scaling
mat4 sca = mat4( 1. , 0. , 0. , 0.,
                 0. , 1. , 0. , 0.,
                 0. , 0. , 1. , 0.,
                 0. , 0. , 0. , 1.);

// Rotation
mat4 rot = mat4( 1. , 0. , 0. , 0.,
                 0. , 1. , 0. , 0.,
                 0. , 0. , 1. , 0.,
                 0. , 0. , 0. , 1.);


void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0) * rot * sca * mov;
}
