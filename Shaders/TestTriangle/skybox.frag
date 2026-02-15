#version 450

// Inputs from vertex shader
layout(location = 0) in vec4 in_fragLocalPos;
layout(location = 1) in vec2 in_normal;

// Uniforms from host
layout(binding = 1) uniform samplerCube envMap;

// Output interface
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 direction = normalize(in_fragLocalPos.xyz);
    outColor = texture(envMap, direction);
}