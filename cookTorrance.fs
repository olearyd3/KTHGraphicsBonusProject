#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// texture maps needed for PBR
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// light positions and colours -- passed in from main
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// camera position for calulcations
uniform vec3 camPos;

// define PI
const float PI = 3.14159265359;

// Function to calculate the normals from a normal map using tangents
vec3 getNormalFromMap() {
    // get the tangent normals
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    // calculate derivatives for fragment positions and texcoords
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    // get normal, tangent and the negative of their cross to produce TBN
    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    // multiply TBN by tangent Normal and normalize it to do normal mapping
    return normalize(TBN * tangentNormal);
}

// Normal Distribution GGX - Trowbridge-Reitz GGX
float NormalDistributionGGX(vec3 N, vec3 H, float roughness) {
    // define a to be the roughness squared and square it again for the numerator of the NDF
    float a = roughness*roughness;
    float a2 = a*a;
    // get the dot of the halfvector and the normal (passed in) and square them (part of denominator)
    float NH = max(dot(N, H), 0.0);
    float NH2 = NH*NH;
    // multiply N dot H squared by alpha squared -1 and add 1 to it 
    float denom = (NH2 * (a2 - 1.0) + 1.0);
    // multiply by PI and square the denominator to get final denominator
    denom = PI * denom * denom; 

    return a2/denom;
}

// Geometry Schlick GGX
float GeometrySchlickGGX(float NV, float roughness) {
    // since direct lighting is being used, k is a+1 squared then divided by 8
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    // the numerator is N dot V and the denominator is N dot V times 1-k + k
    float numerator= NV;
    float denominator = NV * (1.0 - k) + k;

    return numerator / denominator;
}

// Geometry Smith - use the result of Geometry Schlick GGX to take geometry obstruction and shadowing into account
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    // get the dot product of N with V and N with L again
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    // call the SchlickGGX for NdotV and for NdotL and multiply their results 
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel Schlick - describes ratio of reflection over refraction
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    // fresnel-schlick approximation formula where F0 is set to constant value of 0.04
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {		
    // define each of the maps being read in 
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;

    // calculate the normal from normal map and view vector 
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance -- if dielectric use F0 of 0.04. If metal, use albedo colour
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // for each light
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) {
        // calculate the light and half vectors, the distance and the attenuation
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        // calculate radiance as light colour time attenuation
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF is defined using NDF, G and F - call each function 
        float NDF = NormalDistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        // to get the specular component: numerator is simply DFG
        vec3 numerator    = NDF * F * G; 
        // denominator is 4 times N dot V time N dot L (+ 0.0001 is to prevent any division by zero)
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        // kS is equal to the result of = Fresnel Schlick
        vec3 kS = F;
        // for energy conservation required for PBR, diffuse and specular light can'tbe above 1.0 (unless the surface emits light)
        // so, diffuse component equals 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse of metallic so non-metals only have diffuse lighting,
        kD *= 1.0 - metallic;	  

        // scale light by the N dot L
        float NdotL = max(dot(N, L), 0.0);        

        // BRDF equation: (already multiplied by kS since this equals F so do not need to include here)
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   
    
    // adding ambient light by multiplying the albedo and ao by a small vector so not too bright
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // output colour is ambient light plus result of BRDF
    vec3 color = ambient + Lo;

    // HDR tone mapping
    color = color / (color + vec3(1.0));

    // gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    // output the final colour for each fragment 
    FragColor = vec4(color, 1.0);
}