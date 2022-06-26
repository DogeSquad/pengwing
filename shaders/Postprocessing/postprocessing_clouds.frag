#version 330 core
out vec4 FragColor;
  

// Post Processing
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float aspectRatio;
const vec2 SCREEN_SIZE = vec2(1920, 1080);

// Ray marching
const int MAX_MARCHING_STEPS = 250;
const float GRAD_STEP = 0.0005f;
uniform vec3 view_pos;
uniform vec3 light_pos;

//float sdBox( vec3 p, vec3 b )
//{
//    vec3 q = abs(p) - b;
//    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
//}
float map(vec3 p)
{
    vec3 spherePosition = vec3(0.0f, 0.0f, 0.0f);
    float d = distance(p, spherePosition.xyz) - 1.;     // sphere at (-1,0,5) with radius 1
    return d;
}
vec3 calcNormal(in vec3 p) {
    vec2 e = vec2(1.0f, -1.0f) * GRAD_STEP;
    return normalize(
        e.xyy * map(p + e.xyy) +
        e.yyx * map(p + e.yyx) +
        e.yxy * map(p + e.yxy) +
        e.xxx * map(p + e.xxx));
}

bool raymarch(vec3 ro, vec3 rd, inout float t)
{
    float h = 1.0f;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        h = map(ro + rd * t);
        t += h;
        if (h < 0.01f) break;
    }
    return h < 0.01f;
}

void main()
{           
    vec3 ro = view_pos;
    vec2 q = (gl_FragCoord.xy - 0.5f * SCREEN_SIZE) / SCREEN_SIZE.y;
    vec3 rd = normalize(vec3(q, -1.0f) - ro);
    float t = 1.0f;
    if (raymarch(ro, rd, t)) 
    {
        vec3 p = ro + rd * t;
        vec3 normal = calcNormal(p);
        
        // Calculate diffuse lighting by taking the dot product of 
        // the light direction (light-p) and the normal.
        float dif = clamp(dot(normal, normalize(light_pos - p)), 0.0f, 1.0f);
		
        // Multiply by light intensity (5) and divide by the square
        // of the distance to the light.
        dif *= 5. / dot(light_pos - p, light_pos - p);
        
        FragColor = vec4(vec3(pow(dif, 0.4545f)), 1.0f);     // Gamma correction
        //return;
    }
    FragColor = texture(screenTexture, TexCoords);
}