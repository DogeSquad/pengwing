#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

in vec3 viewOrigin;
in vec3 viewDir;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform sampler3D perlinNoise;
uniform sampler3D worleyNoise;
uniform vec2 uRes;

uniform int frame;

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform float near;
uniform float far;

uniform vec3 viewPos;

uniform vec3 boundsMin;
uniform vec3 boundsMax;

// Clouds (adapted from Sebastian Lague https://www.youtube.com/watch?v=4QOcCGI6xOU)
uniform float CloudScale;
uniform vec3 CloudOffset;
uniform float DensityThreshold;
uniform float DensityMultiplier;

uniform float DarknessThreshold;
uniform float LightAbsorption;
uniform float PhaseVal;

// Ray marching
const int NUM_STEPS = 200;
const int NUM_STEPS_LIGHT = 15;
uniform vec3 lightPos;
// Noise functions: https://github.com/ashima/webgl-noise

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
    float worleyNoiseSample = texture(worleyNoise, uvw).r;

    float density = (texture(perlinNoise, uvw + 0.1f * vec3(worleyNoiseSample)).r + 0.5f * worleyNoiseSample - DensityThreshold) * DensityMultiplier;
    //float density = texture(perlinNoise, uvw).r;
    //float density = max(0.0f, (0.5f * fbm(uvw, 7)) - DensityThreshold) * DensityMultiplier;
    //float density = fbm(uvw, 7);
    return density;
}

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0f * d - 1.0f;
    return 2.0f * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

float lightmarch(vec3 pos)
{
    vec3 dirToLight = lightPos;
    float dstInsideBox = intersectAABB(boundsMin, boundsMax, pos, dirToLight).y;

    float stepSize = dstInsideBox/NUM_STEPS_LIGHT;
    float totalDensity = 0.0f;

    for (int i = 0; i < NUM_STEPS_LIGHT; i++)
    {
        pos += dirToLight * stepSize;
        totalDensity += max(0, sampleDensity(pos) * stepSize);
    }

    float transmittance = exp(-totalDensity * LightAbsorption);
    return DarknessThreshold + transmittance * (1 - DarknessThreshold);
}

void main()
{
    // Calculate Ray
    vec2 pos = (gl_FragCoord.xy - vec2(uRes.xy) * 0.5f) / float(uRes.x);
    vec4 col = texture(screenTexture, TexCoords.xy);

    mat3 rot = mat3(view_mat[0][0], view_mat[1][0], view_mat[2][0],
               view_mat[0][1], view_mat[1][1], view_mat[2][1],
               view_mat[0][2], view_mat[1][2], view_mat[2][2]);

    vec3 rayPos = viewPos;
    vec3 rayDir = (vec4(vec3(pos.xy, 1.0f), 0.0f) * proj_mat * view_mat).xyz;
    vec3 rayDir_norm = (normalize(rayDir)).xyz;
    
    float nonLinearDepth = texture(depthTexture, TexCoords.xy).r;
    float depth = linearize_depth(nonLinearDepth, near, far) * length(rayDir);

    // Perform Ray intersection
    vec2 rayBoxInfo = intersectAABB(boundsMin, boundsMax, rayPos, rayDir_norm);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;
    vec3 entryPoint = rayPos + rayDir * dstToBox;
    
    float dstTravelled = 0.0f;
    float dstLimit = min(depth - dstToBox, dstInsideBox);
    float stepSize = dstInsideBox / NUM_STEPS;
    float transmittance = 1.0f;
    float lightEnergy = 0.0f;

    FragColor = texture(screenTexture, TexCoords.xy);;
    vec4 cloudCol = vec4(1.0f);
    if (dstToBox <= dstInsideBox && dstToBox < depth)
    {
        while (dstTravelled < dstLimit) {
            rayPos = entryPoint + rayDir * dstTravelled;
            float density = sampleDensity(rayPos);
                    
            if (density > 0.0f) {
                float lightTransmittance = lightmarch(rayPos);
                lightEnergy += density * stepSize * transmittance * lightTransmittance * PhaseVal;
                transmittance *= exp(-density * stepSize);
                    
                // Exit early if T is close to zero as further samples won't affect the result much
                if (transmittance < 0.01f) {
                    break;
                }
            }
            dstTravelled += stepSize;
        }
        FragColor = mix(cloudCol * lightEnergy, FragColor, transmittance);
    }

    //FragColor = col;
    //if (dstToBox <= dstInsideBox && dstToBox < depth)
    //{
    //    FragColor = mix(texture(screenTexture, TexCoords.xy), vec4(1.0f), sampleDensity(rayOrigin + rayDir_norm * dstToBox));
    //}


    // Ray Direction Debug
    //vec4 lines = vec4(((rayDir.x > -0.001f && rayDir.x < 0.001f) 
    //                || (rayDir.y > -0.001f && rayDir.y < 0.001f) 
    //                || (rayDir.z > -0.001f && rayDir.z < 0.001f)) ? 1.0f : 0.0f);
    //FragColor = texture(screenTexture, TexCoords.xy);
    //FragColor = mix(vec4(0.5f * rayDir + vec3(0.5f), 1.0f), FragColor, 0.7f);
    //FragColor = mix(lines, FragColor, 1.0f -lines.w);

    
    //float dstTravelled = 0.0f;
    //
    //float totalDensity = 0.0f;
    //float stepSize = dstInsideBox / NUM_STEPS;
    //float dstLimit = dstInsideBox;
    //while(dstTravelled < dstLimit)
    //{
    //    vec3 rayPos = rayOrigin + rayDir * (dstToBox + dstTravelled);
    //    totalDensity += sampleDensity(0.05f * rayPos) * stepSize;
    //    dstTravelled += stepSize;
    //}
    //float transmittance = exp(-totalDensity);
    //FragColor = mix(vec4(1.0f), texture(screenTexture, TexCoords.xy), transmittance);
}