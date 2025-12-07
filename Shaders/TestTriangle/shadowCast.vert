#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;


// Outputs to Fragment shader don't need to be setup in Vulkan Code.
// These values are automatically passed over to Fragment Shader 
// Fragment shader will receive lerped values
layout(location = 0) out vec4 fragWorldPos;
layout(location = 1) out vec4 outNormal;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

void main()
{
    //gl_Position = vec4(in_position.xy, 0.0, 1.0);
    
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(in_position.xyz, 1.0);

    //gl_Position = mvp.proj * mvp.view * fragWorldPos;
}