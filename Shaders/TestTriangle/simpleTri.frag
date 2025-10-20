#version 450

layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec4 viewPosWS;

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

layout(binding = 1) uniform Lights
{
    GlobalLight globalLight;
}lights;

layout(binding = 2) uniform MaterialUniform
{
    Material data;
}material;

//layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    float ambientStrength = 0.1;
    float specularStrength = 0.5;
    float globalLightIntensity = 0.5;

    vec3 inColor = fragColor;

    GlobalLight gLight = lights.globalLight;

    vec3 ambient = gLight.color.xyz * ambientStrength;

    float diff = max(dot(inNormal.xyz, normalize(-gLight.direction)), 0.0);
    vec3 diffuse = diff * globalLightIntensity * gLight.color.xyz;

    //Specular calculation. This is view-dependent
    vec3 viewDir = normalize(viewPosWS - fragWorldPos).xyz;
    vec3 reflectDir = reflect(normalize(gLight.direction), inNormal.xyz);
    float specPower = pow(max(dot(viewDir, reflectDir), 0), material.data.shininess);
    vec3 specular = specularStrength * specPower * gLight.color.xyz;  


    vec4 finalColor = vec4((ambient + specular + diffuse), 1.0f);
    outColor = finalColor;
    

    //float dot = dot(reflectDir, inNormal.xyz);
    //outColor = vec4(dot, dot, dot, 1.0f);

    //outColor = vec4(inNormal.xyz, 1.0f);
    //outColor = texture(texSampler, fragTexCoord);
}