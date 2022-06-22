#version 330 core

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
} vs_out;

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform Material material;
void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(0.3f, 0.3f, 0.3f);
  	
    // diffuse 
    vec3 norm = normalize(vs_out.Normal);
    vec3 lightDir = normalize(lightPos - vs_out.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(material.texture_diffuse2, vs_out.TexCoords).xyz * vec3(0.9f, 0.9f, 0.8f);
    
    // specular
    vec3 viewDir = normalize(viewPos - vs_out.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = texture(material.texture_specular1, vs_out.TexCoords).xyz * spec * vec3(0.9f, 0.9f, 0.8f);  
        
    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
} 