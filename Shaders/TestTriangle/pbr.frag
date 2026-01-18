#version 450

layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec4 viewPosWS;
layout(location = 5) in vec4 inLightProjPos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

struct GlobalLight
{
    vec3 direction;
    vec4 color;
};

struct PointLight
{
    vec3 position;
    vec3 direction;
    vec4 color;
    float radius;
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

struct PBRProperties
{
    vec4 albedo;
    float metallic;
    float roughness;
};

layout(binding = 2) uniform Lights
{
    GlobalLight globalLight;
}lights;

layout(binding = 3) uniform sampler2D shadowMap;

//PBR stuff
//TODO: Move these to a new specularStrength
//TODO: Convert to a texture array?
layout(binding = 4) uniform PBR
{
    PBRProperties data;
} pbr;

layout(binding = 5) uniform sampler2D diffuseMap;
layout(binding = 6) uniform sampler2D normalMap;
layout(binding = 7) uniform sampler2D metallicMap;
layout(binding = 8) uniform sampler2D roughnessMap;


//layout (input_attachment_index=0, binding=3) uniform subpassInput myInputAttachment;

//layout(binding = 1) uniform sampler2D texSampler;
vec3 CalculateFresnel(vec3 h, vec3 v, vec3 albedo, float metallic);
float DistrubutionGGX_TrowbridgeReitz(vec3 h, vec3 n, float roughness);
float GeometryGGX_Smith(vec3 n, vec3 v, vec3 l, float roughness);

float calculateShadow(vec4 lightProjCoords)
{
    //Perspective divide
    vec3 projCoords = lightProjCoords.xyz / lightProjCoords.w;

    //Convert to range [0, 1]
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * 0.5 + 0.5;
    
    //NOTE: The current values are probably not in range[-1 1]. Investigate!!!!!
    //projCoords.z = projCoords.z * 0.5 + 0.5;

    //Sample from the shadowMap
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // 1: is in shadow; 0: not in shadow
    float bias = 0.00005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    const float ambientStrength = 0.1f;
    

    //vec3 albedo = texture(diffuseMap, fragTexCoord).xyz;
    vec3 albedo = vec3(1.0f);

    float shadow = calculateShadow(inLightProjPos);

    /*
        Light observed is of 2 parts: 
        1. Refracted(Kd)
        2. Reflected(Ks) (Calculated by BRDF)
    */
    GlobalLight gl = lights.globalLight;
    vec3 ambientLight = gl.color.xyz * ambientStrength;

    vec3 lightDirection = normalize(-gl.direction);
    vec3 normal = inNormal.xyz;
    //normal = texture(normalMap, fragTexCoord).xyz;
    //normal = vec3(normal.x, normal.z, normal.y);
    //normal = normalize(normal);
    

    vec3 phi = gl.color.xyz; // Represents radiant flux
    vec4 p = fragWorldPos;
    vec3 wi = lightDirection;
    vec3 wo = normalize(viewPosWS - fragWorldPos).xyz;
    float ndotl = max(dot(normal, wi), 0);
    float ndotv = max(dot(normal, wo), 0);
    vec3 halfwayVector = (wi + wo) / length(wi + wo);

    /* Radiance calculation
    - Radiance is a function of position of fragment and incoming light ray
    - L(p, wi): 
        p would have to represent an area in the reflectance equation, but for single point light source, it would only affect a single point
        wi would have to w, which represents a solid angle. It's the area projected onto a unit sphere.
        This is basically the area of the light projected onto the unit sphere.
        The solid angle is assumed to be infinitesimally small, which would be a single incoming light vector
        L = (d^2phi) / (dAdw cosTheta)
        We assume radiant flux(phi) to be the light color. 
        dA is infinitesimally smally and only represents a point. dw represents only the direction to incoming light.
        As the area is inifinitesimally small, cosTheta with incoming light direction can also be ignored(not sure)
        L essentially just becomes radiant flux(light color)!!

        Since we know beforehand the location of all light sources while shading a single point, it's not needed to solve the integral for irradiance.
        We have to take integral into account for IBL
    */

    float attenuation = 1; //TODO: Matters if light's radiance should be scaled over distance. Doesn't matter for point lights.

    vec3 radiance = phi * attenuation * ndotl;


    /* BRDF
    - Consists of 3 parts: 
        1. Normal Distribution function: 
            Statistically approximates the amount of surface microfacets aligned to halfway vector
        2. Geometry function:
            Describes self-shadowing property of microfacets. 
            When surface is relatively rough, microfacets can overshadow other microfacets, reducing the light reflected` 
        3. Fresnel equation: 
            Describes ratio of surface reflection at different surface angles
    */

    //float metallic = texture(metallicMap, fragTexCoord).r;
    //float roughness = texture(roughnessMap, fragTexCoord).r;

    float metallic = pbr.data.metallic;
    float roughness = pbr.data.roughness;

    vec3 F = CalculateFresnel(halfwayVector, wo, albedo, metallic);
    float D = DistrubutionGGX_TrowbridgeReitz(halfwayVector, normal, roughness);
    float G = GeometryGGX_Smith(normal, wo, wi, roughness);

    float denom = (4 * ndotl * ndotv) + 0.001f;
    vec3 specularWo = (F * D * G / denom);

    vec3 ks = F;
    vec3 kd = vec3(1.0f) - ks;
    //If surface is metallic, no light is refracted. We enforce this using the metallic surface parameter
    kd *= (1.0f - metallic);

    vec3 diffuseWo = kd * (albedo / PI);

    vec3 radianceOutput = (diffuseWo + specularWo) * radiance;

    //vec3 lightColor = ambientLight + radiance * (1.0 - shadow);
    //outColor = vec4(albedo * lightColor, 1.0f);

    //outColor = vec4(F, 1.0f);
    outColor = vec4(D, D, D, 1.0f);
    //outColor = vec4(G, G, G, 1.0f);
    //outColor = vec4(specularWo, 1.0f);
    //outColor = vec4(radianceOutput, 1.0f);
    
    //outColor = vec4(radiance, 1.0f);
    //outColor = vec4(ndotl, ndotl, ndotl, 1.0f);
    //outColor = vec4(diffuseWo, 1.0f);
}


vec3 FresnelSchlick(vec3 f0, vec3 h, vec3 v)
{
    float angle = max(dot(h, v), 0);
    return f0 + (1 - f0) * pow(1 - angle, 5);
}

vec3 CalculateFresnel(vec3 h, vec3 v, vec3 albedo, float metallic)
{
    // Fresnel equation will give the ratio of light reflected. It varies over the given view angle to halfway vector.
    // For Dielectric materials, the reflected light is baseReflectivity + additional reflection contribution at an angle. 
    // For Metallic materials, the reflected light is tinted with the surface's color.
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);
    return FresnelSchlick(f0, h, v);
}

float DistrubutionGGX_TrowbridgeReitz(vec3 h, vec3 n, float roughness)
{
    // Statistically approximates the relative surface area of microfacets aligned to the halfway vector

    // If surface is rough, microfacet area aligned to the halfway vector is lesser
    // For roughness = 0, there would ideally be a single point that aligns to the halfway vector

    float rSqr = roughness * roughness * roughness * roughness;
    float dp = max(dot(n, h), 0);

    float denom = ((rSqr - 1) * dp * dp + 1);

    return rSqr / (PI * denom * denom);
}

float GeometryGGX_Schlick(float ndotv, float k)
{
    //Idea is that as roughness increases, the area of overshadowing increases which makes surface look darker
    // ((ndotv) * (1.0f - k) + k) should be clamped to [0, 1] if k is clamped to [0, 1]

    return ndotv / ((ndotv) * (1.0f - k) + k);
}

float GeometryGGX_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float k = (roughness + 1) * (roughness + 1) / 8.0f;
    float ndotl = max(dot(n, l), 0);
    float ndotv = max(dot(n, v), 0);

    // Geometry shadowing from light source
    float ggx1 = GeometryGGX_Schlick(ndotl, k);
    // Geometry obstruction to view vector
    float ggx2 = GeometryGGX_Schlick(ndotv, k);
    return ggx1 * ggx2;
}

/*Questions:
- What is GGX?

*/
