#version 450

layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D sceneRender;

layout(binding = 1) uniform samplerCube testCubeMap;

void main()
{
    outColor = texture(sceneRender, texcoord);
    //outColor = texture(testCubeMap, vec3(texcoord, 0.0f));
}