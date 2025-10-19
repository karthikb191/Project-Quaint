#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec4 in_color;


// Outputs to Fragment shader don't need to be setup in Vulkan Code.
// These values are automatically passed over to Fragment Shader 
// Fragment shader will receive lerped values
layout(location = 0) out vec4 fragWorldPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out vec4 outViewPosWs;

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

    fragWorldPos = mvp.model * vec4(in_position.xyz, 1.0);
    fragColor = in_color.xyz;
    fragTexCoord = in_texcoord;
    outNormal.xyz = mat3(mvp.model) * in_normal.xyz;
    outNormal = vec4(normalize(outNormal.xyz), 1.0f);

    outViewPosWs.x = -dot(mvp.view[0], mvp.view[3]);
    outViewPosWs.y = -dot(mvp.view[1], mvp.view[3]);
    outViewPosWs.z = -dot(mvp.view[2], mvp.view[3]);
    outViewPosWs.w = 1;

    //gl_Position = mvp.proj * mvp.view * fragWorldPos;
}