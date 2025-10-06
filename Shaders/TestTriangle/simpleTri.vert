#version 450

//layout(location = 0) in vec2 in_position;
//layout(location = 1) in vec3 in_color;
//layout(location = 2) in vec2 in_texcoord;


layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_texcoord;


// Outputs to Fragment shader don't need to be setup in Vulkan Code.
// These values are automatically passed over to Fragment Shader 
// Fragment shader will receive lerped values
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    //gl_Position = vec4(in_position.xy, 0.0, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position.xyz, 1.0);
    fragColor = in_color.xyz;
    fragTexCoord = in_texcoord;
}