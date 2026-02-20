#version 450

// Inputs from vertex shader
layout(location = 0) in vec4 in_fragLocalPos;
layout(location = 1) in vec2 in_normal;

// Uniforms from host
layout(binding = 1) uniform samplerCube envMap;

// Output interface
layout(location = 0) out vec4 outColor;

vec3 sampleIrradiance(vec3 direction)
{
    return vec3(1.0f);
}

void main()
{
    vec3 irradianceRes = sampleIrradiance(in_fragLocalPos.xyz);
    outColor = vec4(irradianceRes, 1.0f);
}