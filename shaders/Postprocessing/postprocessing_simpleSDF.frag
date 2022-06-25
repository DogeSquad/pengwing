#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float aspectRatio;

float sdf_Circle(vec2 pos, float radius)
{
    return length(pos) - radius;
}

void main()
{           
    float radius = 0.9f;

    // transform coords to ndc
    vec2 ndc = 2.0f * TexCoords - vec2(1.0f);
    ndc = vec2(ndc.x * aspectRatio, ndc.y);

    // post process coord
    if (sdf_Circle(ndc, radius) <= 0.0f) {
        FragColor=texture(screenTexture, TexCoords);
        return;
    }
    FragColor = mix(texture(screenTexture, TexCoords), vec4(vec3(0.0f), 1.0f), 0.6f);
}