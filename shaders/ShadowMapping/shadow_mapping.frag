#version 330 core

uniform sampler2D shadowMap;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_specular1;
};

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;


uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform Material material;

float ShadowCalculation(vec4 fragPosLightSpace, float dotNL)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    //float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.0f;
    bias = max(0.3f * (1.0f - dotNL), 0.2f);
    //float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f; 
    //if(projCoords.z > 1.0f)
    //    shadow = 0.0f;
    //
    //return shadow;

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

void main()
{
    vec3 lightColor = vec3(0.9f, 0.9f, 0.8f);
    vec3 color = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 lightDir = normalize(lightPos);
    vec3 normal = normalize(fs_in.Normal);
    float dotNL = dot(lightDir, normal);

    // ambient
    vec3 ambient = 0.15f * lightColor;

    // diffuse
    float diff = max(dotNL, 0.0f);
    vec3 diffuse = diff * lightColor;

    // Calculate Blinn-Phong
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = texture(material.texture_specular1, fs_in.TexCoords).r;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, dotNL);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
} 