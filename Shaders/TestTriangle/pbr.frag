#version 450

layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec4 viewPosWS;
layout(location = 5) in vec4 inLightProjPos;

layout(location = 0) out vec4 outColor;


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

layout(binding = 2) uniform Lights
{
    GlobalLight globalLight;
}lights;

layout(binding = 3) uniform sampler2D shadowMap;

//PBR stuff
//TODO: Move these to a new specularStrength
//TODO: Convert to a texture array?
layout(binding = 4) uniform sampler2D diffuseMap;
layout(binding = 5) uniform sampler2D normalMap;
layout(binding = 6) uniform sampler2D metallicMap;
layout(binding = 7) uniform sampler2D roughnessMap;


//layout (input_attachment_index=0, binding=3) uniform subpassInput myInputAttachment;

//layout(binding = 1) uniform sampler2D texSampler;


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
    vec3 diffuse = texture(diffuseMap, fragTexCoord).xyz;

    float shadow = calculateShadow(inLightProjPos);
    vec4 finalColor = vec4((diffuse * (1.0 - shadow)).xyz, 1.0f);
    outColor = finalColor;
}
