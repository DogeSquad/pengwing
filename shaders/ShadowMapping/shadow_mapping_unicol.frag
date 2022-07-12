#version 330 core

out vec4 FragColor;

uniform sampler2D shadowMap;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform float near;
uniform float far;
uniform vec3 fogColor;

uniform float minBias;
uniform float maxBias;

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0f * d - 1.0f;
    return 2.0f * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}
float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;
    float currentDepth = projCoords.z;
    
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(maxBias * (1.0f - dot(normal, lightDir)), minBias);
    //float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f;
    
    if(projCoords.z > 1.0f)
        return 0.0f;

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;        
        }    
    }
    shadow /= 9.0f;
    return shadow;
}
float getFogFactor(float d)
{
    float farMultiplier = 1.05f;
    float newFar = far * farMultiplier;
    if (d>=newFar) return 1;
    if (d<=near) return 0;

    return 1 - (newFar - d) / (newFar - near);
}

void main()
{
    vec3 color = vec3(0.8f, 0.8f, 0.8f);
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = 0.15f * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos);
    float diff = max(dot(lightDir, normal), 0.0f);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 64.0f);
    spec *= 0.2f;
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0f - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0f);
    FragColor = mix(FragColor, vec4(fogColor, 1.0f), pow(getFogFactor(distance(viewPos, fs_in.FragPos)), 5));
} 