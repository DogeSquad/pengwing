#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

in vec3 viewOrigin;
in vec3 viewDir;

uniform sampler2D screenTexture;
//uniform sampler2D depthTexture;
//uniform sampler3D perlinNoise;
uniform vec2 uRes;

uniform int frame;

uniform mat4 proj_mat;
uniform mat4 view_mat;

uniform vec3 viewPos;

uniform vec3 boundsMin;
uniform vec3 boundsMax;

// Ray marching
const int NUM_STEPS = 50;
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

//_---------------------------

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159f - 0.85373472095314f * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0f/6.0f, 1.0f/3.0f) ;
  const vec4  D = vec4(0.0f, 0.5f, 1.0f, 2.0f);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0f - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0f, i1.z, i2.z, 1.0f ))
           + i.y + vec4(0.0f, i1.y, i2.y, 1.0f )) 
           + i.x + vec4(0.0f, i1.x, i2.x, 1.0f ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857f; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0f * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0f - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0f + 1.0f;
  vec4 s1 = floor(b1)*2.0f + 1.0f;
  vec4 sh = -step(h, vec4(0.0f));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }
//_---------------------------
float fbm(vec3 pos, int octaves)
{
    float lacunarity = 2.0f;
    float gain = 0.5f;
    // Initial values
    float amplitude = 0.5f;
    float frequency = 1.0f;
    // Loop of octaves
    float total = 0.0f;
    for (int i = 0; i < octaves; i++) {
	    total += amplitude * snoise(frequency*pos);
	    frequency *= lacunarity;
	    amplitude *= gain;
    }
    return total;
}
float sampleDensity(vec3 pos)
{
    float density = fbm(pos + vec3(frame * 0.0005f, frame * 0.0005f, 0.0f), 6);
    return density;
}

void main()
{
    // Calculate Ray
    vec2 pos = (gl_FragCoord.xy - vec2(uRes.xy) * 0.5f) / float(uRes.x);
    vec4 col = texture(screenTexture, TexCoords.xy);

    mat3 rot = mat3(view_mat[0][0], view_mat[1][0], view_mat[2][0],
               view_mat[0][1], view_mat[1][1], view_mat[2][1],
               view_mat[0][2], view_mat[1][2], view_mat[2][2]);

    vec3 rayOrigin = viewPos;
    vec3 rayDir = (normalize(vec4(vec3(pos.xy,1), 0.0f)) * proj_mat * view_mat).xyz;
    
    vec2 rayBoxInfo = intersectAABB(boundsMin, boundsMax, rayOrigin, rayDir);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;
    
    FragColor = col;
    if (dstToBox <= dstInsideBox)
    {
        FragColor = mix(texture(screenTexture, TexCoords.xy), vec4(1.0f), sampleDensity(rayOrigin + rayDir * dstToBox));
    }
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