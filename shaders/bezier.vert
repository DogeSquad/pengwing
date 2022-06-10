#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform uvec2 uRes;
uniform float uTime;
uniform uvec2 uPos[4];

out vec4 interp_color;
out vec3 interp_normal;  

void main()
{   
    float ar = float(uRes.x) / float(uRes.y);
    mat4 sca_mat = mat4(0);
    sca_mat[0][0] = sca_mat[1][1] = sca_mat[2][2] =  0.2;
    sca_mat[1][1] *= ar;
    sca_mat[3][3] = 1.;
    gl_Position = sca_mat * proj_mat * view_mat * model_mat * vec4(position.x, position.y, position.z, 1.0);
    interp_color = color;
    interp_normal = normalize((transpose(inverse(model_mat)) * vec4(normal, 0.0)).xyz);
}
