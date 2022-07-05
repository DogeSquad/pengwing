#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// Frame Info
uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform sampler3D perlinNoise;
uniform sampler3D worleyNoise;
uniform vec2 uRes;
uniform int frame;

// Render Info
uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform vec3 viewPos;
uniform float near;
uniform float far;


// Clouds (adapted from Sebastian Lague https://www.youtube.com/watch?v=4QOcCGI6xOU) 
//                  and Adam Bengtsson https://www.diva-portal.org/smash/get/diva2:1647354/FULLTEXT02)

uniform vec3 boundsMin;
uniform vec3 boundsMax;

uniform float CloudScale;
uniform vec3 CloudOffset;
uniform float DensityThreshold;
uniform float DensityMultiplier;

uniform float DarknessThreshold;
uniform float LightAbsorption;
uniform float PhaseVal;

uniform vec3 CloudColor;

// Ray marching
const int NUM_STEPS = 200;
const int NUM_STEPS_LIGHT = 10;
uniform vec3 lightPos;

vec2 intersectAABB(vec3 boxMin, vec3 boxMax, vec3 rayOrigin, vec3 rayDir) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}
float sampleDensity(vec3 pos)
{
    vec3 uvw = pos * CloudScale * 0.001f + CloudOffset * 0.01f;

    float perlinNoiseSample = texture(perlinNoise, vec3(2.0f * uvw.x, uvw.yz)).r;
    float worleyNoiseSample = texture(worleyNoise, uvw + 0.6f * vec3(perlinNoiseSample)).r;

    float density = max(0.0f, (worleyNoiseSample + 0.6f * perlinNoiseSample - DensityThreshold) * DensityMultiplier);
    return density;
}

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0f * d - 1.0f;
    return 2.0f * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}
float col_ramp(float fac, float min, float max, float val_min, float val_max)
{
    float m = (val_max - val_min)/(max-min);
    float b = -min * m;
    return (fac < min) ? val_min : ((fac > max) ? val_max : fac*m+b);
}
float lightmarch(vec3 pos)
{
    vec3 dirToLight = lightPos - pos;
    float dstInsideBox = intersectAABB(boundsMin, boundsMax, pos, dirToLight).y;

    float stepSize = dstInsideBox/NUM_STEPS_LIGHT;
    float totalDensity = 0.0f;

    for (int i = 0; i < NUM_STEPS_LIGHT; i++)
    {
        pos += dirToLight * stepSize;
        totalDensity += max(0.0f, sampleDensity(pos) * stepSize);
    }

    float transmittance = exp(-totalDensity * LightAbsorption);
    return DarknessThreshold + transmittance * (1.0f - DarknessThreshold);
}

void main()
{
    FragColor = texture(screenTexture, TexCoords.xy);

    // Calculate Ray
    vec2 pos = (gl_FragCoord.xy - vec2(uRes.xy) * 0.5f) / float(uRes.x);

    vec3 rayPos = viewPos;
    vec3 rayDir = (vec4(vec3(pos.xy, 1.0f), 0.0f) * proj_mat * view_mat).xyz;
    vec3 rayDir_norm = normalize(rayDir);
    
    float depth = linearize_depth(texture(depthTexture, TexCoords.xy).r, near, far) * length(rayDir);

    // Perform Ray intersection
    vec2 rayBoxInfo = intersectAABB(boundsMin, boundsMax, rayPos, rayDir_norm);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;
    vec3 entryPoint = rayPos + rayDir_norm * dstToBox;
    
    float dstTravelled = 0.0f;
    float dstLimit = min(depth - dstToBox, dstInsideBox);
    float stepSize = dstInsideBox / NUM_STEPS;
    float transmittance = 1.0f;
    float lightEnergy = 0.0f;
    
    if (dstToBox <= dstInsideBox && dstToBox < depth)
    {
        while (dstTravelled < dstLimit) {
            rayPos = entryPoint + rayDir * dstTravelled;
            float density = sampleDensity(rayPos);
                    
            if (density > 0.0f) {
                float lightTransmittance = lightmarch(rayPos);
                lightEnergy += density * stepSize * transmittance * lightTransmittance * PhaseVal;
                transmittance *= exp(-density * stepSize);
                    
                if (transmittance < 0.01f) {
                    break;
                }
            }
            dstTravelled += stepSize;
        }
        FragColor = mix(vec4(CloudColor, 1.0f) * lightEnergy, FragColor, col_ramp(transmittance,0.1f, 0.9f,  0.0f, 1.0f));
    }
}