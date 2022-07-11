#version 330 core
#define M_PI 3.1415926535897932384626433832795f


out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 skyColor;
uniform vec3 fogColor;
uniform vec3 sunColor;
uniform vec2 uRes;

uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform vec3 viewPos;
uniform vec3 sunPos;

float gradFunc(float x) 
{
    if (x < 0.0f) return 1.0f;
    return pow(cos(0.5f * M_PI * x), 12);
}
float greatCircleDistance(vec3 a, vec3 b)
{
    float euclideanDst = distance(a, b);
    float angle = asin(euclideanDst * 0.5f);  // Assumed R is 1.0f

    return acos(dot(a, b));

    //return 2.0f * angle;
}

void main()
{   
    vec2 pos = (gl_FragCoord.xy - vec2(uRes.xy) * 0.5f) / float(uRes.y);
    vec3 world_pos = (inverse(view_mat) * vec4(vec3(pos.xy, -1.0f), 1.0f) /* * proj_mat * view_mat*/).xyz;

    vec3 rayPos = viewPos;
    vec3 rayDir = world_pos - viewPos;
    rayDir = normalize(rayDir);

    vec3 sunPos_norm = normalize(sunPos);

    if (greatCircleDistance(sunPos_norm, rayDir) < 0.032f) 
    {
        FragColor = vec4(sunColor, 1.0f);
        return;
    }

    FragColor = vec4(mix(skyColor, fogColor, gradFunc(rayDir.y)), 1.0f);
    float sunPow = pow(max(0.0f, 13.0f * dot(rayDir, sunPos_norm) - 12.0f), 8);
    FragColor = mix(FragColor, vec4(sunColor, 1.0f), sunPow);

    //FragColor += vec4(sunColor, 1.0f) * max(0.0f, dot(rayDir, normalize(sunPos)));


    // Ray Direction Debug
    //vec4 lines = vec4(((rayDir.x > -0.001f && rayDir.x < 0.001f) 
    //                || (rayDir.y > -0.001f && rayDir.y < 0.001f) 
    //                || (rayDir.z > -0.001f && rayDir.z < 0.001f)) ? 1.0f : 0.0f);
    //FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //FragColor = mix(vec4(0.5f * rayDir + vec3(0.5f), 1.0f), FragColor, 0.7f);
    //FragColor = mix(lines, FragColor, 1.0f -lines.w);
}  