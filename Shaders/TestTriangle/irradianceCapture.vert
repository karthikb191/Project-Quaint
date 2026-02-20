#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec2 in_normal;

layout(location = 0) out vec4 out_fragLocalPos;
layout(location = 1) out vec2 out_normal;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

void main()
{
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(in_position.xyz, 1.0);
    
    out_fragLocalPos = in_position;
}