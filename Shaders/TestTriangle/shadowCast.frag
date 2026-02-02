#version 450

layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 inNormal;

//layout(location = 0) out vec4 outColor;

void main()
{
    //Empty as writing to depth buffer happens in vulkan pipeline
    //outColor = vec4(1.0f);
}