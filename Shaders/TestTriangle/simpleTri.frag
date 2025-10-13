#version 450

layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragColor;

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

layout(binding = 1) uniform Lights
{
    GlobalLight globalLight;
}lights;

//layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    vec3 inColor = fragColor;

    GlobalLight gLight = lights.globalLight;

    float ambientStrength = 0.1;
    vec3 ambient = gLight.color.xyz * ambientStrength;

    float diff = max(dot(inNormal.xyz, normalize(-gLight.direction)), 0.0);
    vec3 diffuse = diff * gLight.color.xyz;

    vec4 finalColor = vec4((ambient + diffuse), 1.0f);
    outColor = finalColor;

    //outColor = vec4(inNormal.xyz, 1.0f);
    //outColor = texture(texSampler, fragTexCoord);
}