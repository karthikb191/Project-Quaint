#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

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
    vec4 finalColor = vec4(inColor.xyz, 1.0f);

    GlobalLight gLight = lights.globalLight;

    finalColor *= gLight.color;
    outColor = finalColor;

    //outColor = vec4(fragTexCoord, 0.0f, 1.0f);
    //outColor = texture(texSampler, fragTexCoord);
}