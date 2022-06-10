#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec3 interp_light_dir;

out vec4 frag_color;

uniform bool useOrenNayar;

uniform float roughness; // sigma
uniform float refractionIndex;
uniform vec4 diffuse; // diffuse part as color
uniform vec4 specular; // specular part as color

const float pi = 3.14159265359;

// Syntatic sugar. Make sure dot products only map to hemisphere
float cdot(vec3 a, vec3 b) {
    return clamp(dot(a,b), 0.0, 1.0);
}

// D
float beckmannDistribution(float dotNH) {
    float sigma2 = roughness * roughness;
    float alpha = acos(dotNH);

    return exp(-pow(tan(alpha), 2) / sigma2) / (pi * sigma2 * pow(cos(alpha), 4));
}

// F
float schlickApprox(float dotVH, float n1, float n2) {
    float R = pow((n1 - n2) / (n1 + n2), 2);

    return R + (1- R) * pow((1 - dotVH), 5);
}

// G
float geometricAttenuation(float dotNH, float dotVN, float dotVH, float dotNL) {
    return min(1.0f, min(2.0f * dotNH * dotVN / dotVH, 2.0f * dotNH * dotNL / dotVH));
}

float cooktorranceTerm(vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // in eye space direction towards viewer simply is the Z axis
    vec3 h = normalize(l + v); // half-vector between V and L

    // precompute to avoid redundant computation
    float dotVN = cdot(v, n);
    float dotNL = cdot(n, l);
    float dotNH = cdot(n, h);
    float dotVH = cdot(v, h);

    float D = beckmannDistribution(dotNH);
    float F = schlickApprox(dotVH, 1.0, refractionIndex);
    float G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

    return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

float dot_clamp(vec3 a, vec3 b) {
    return clamp(dot(a, b), 0.0f, 1.0f);
}

float orennayarTerm(float lambert, vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // Im eye space ist die Richtung zum Betrachter schlicht die Z-Achse
    float sigma2 = roughness * roughness; // sigma^2

    float A = 1 - 0.5f * sigma2 / (sigma2 + 0.57f);
    float B = 0.45f * sigma2 / (sigma2 + 0.09f);
    
    float theta_L = acos(dot_clamp(l, n));
    float theta_V = acos(dot_clamp(v, n));
    
    float alpha = theta_L;
    float beta = theta_V;
    if (theta_L < theta_V) {
        alpha = theta_V;
        beta = theta_L;
    }

    // Project L
    vec3 proj_unnorm = l - dot_clamp(l, n) * n;
    vec3 l_proj = normalize(proj_unnorm);
    // Project V
    proj_unnorm = v - dot_clamp(v, n) * v;
    vec3 v_proj = normalize(proj_unnorm);

    float cos_azimuth = dot_clamp(l_proj, v_proj);

    float oren_nayar = cos(theta_L) * (A + (B * max(0.f, cos_azimuth) * sin(alpha) * tan(beta)));

    // TASK: implement Oren-Nayar Term and use instead of 1.0 below:
    return lambert * oren_nayar;
}

void main() {
    // Lambertian reflection term
    float diffuseTerm = cdot(interp_normal, interp_light_dir);
    // define the diffuse part to be Lambertian - unless we choose Oren-Nayer
    // in this case compute Oren-Nayar reusing the Lambertian term and use that
    if (useOrenNayar) {
        diffuseTerm = orennayarTerm(diffuseTerm, interp_normal, interp_light_dir);
    }
    // lowest possbile value = ambient fake light term
    diffuseTerm = max(diffuseTerm, 0.1);
    // as specular part we compute the Cook-Torrance term
    float specularTerm = cooktorranceTerm(interp_normal, interp_light_dir);
    // combine both terms (diffuse+specular) using our material properties (colors)
    frag_color = vec4(vec3(clamp(diffuse * diffuseTerm + specular * specularTerm, 0.0, 1.0)), 1);
}
