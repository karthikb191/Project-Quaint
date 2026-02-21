#version 450

// Inputs from vertex shader
layout(location = 0) in vec4 in_fragLocalPos;
layout(location = 1) in vec2 in_normal;

// Uniforms from host
layout(binding = 1) uniform samplerCube envMap;

layout(push_constant) uniform uPushConstant {
    float roughness;
} pc;

// Output interface
layout(location = 0) out vec4 outColor;


float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    // This gives the 2D coordinates(u, v) that represents the azimuth and zenith angles
    // The u is straight forward, which is azimuth angle and just goes around the sphere.
    // Value of v is the one that's slightly randomized 
    // Read more about it here: https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/monte-carlo-methods-in-practice/introduction-quasi-monte-carlo.html

    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    const float PI = 3.14159265f;
    float a = roughness*roughness;

    //Getting phi is obvious. We just need to go around the sphere based on the number of samples.
    float phi = 2.0 * PI * Xi.x;

    // For azimuth, we need to have a very narrow reflection lobe for 0 roughness
    // The reflection lobe would be wider for increasing roughness.
    // This pretty much describes the zenith value.
    // The result from quasi-random sampler gives the length on Z, which is cos(theta), 
    // but we remap it based on roughness parameter to bias the final length of Z(cos(theta)).
    // If Xi.y is 1, it represent the entire length of Z, which means zenith angle is 90 degrees.
    // If Xi.y is 0, it represents 0 length of Z, which means zenith angle is 0 degrees.
    // Therefore, we negate this, i.e., (1 - xi.y) to get the cos value, but scaled based on the roughness parameter.

    // Also, it follows a curve, with lesser roughness values gives result close to 1, 
    // implying higher zeith value and smaller reflection lobe
    // For values with higher reflection, the lobe should be wider, the zenith angle should be larger,
    // so, the result is larger

    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    // This condition is just to ensure we don't get a gimbal lock as up is the direction of the normal oriented along Z-axis
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

vec3 prefilterEnvMap(vec3 direction)
{
    const uint SAMPLE_COUNT = 4096u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);   
    }

    return vec3(1.0f);
}

void main()
{
    vec3 prefilterRes = prefilterEnvMap(in_fragLocalPos.xyz);
    outColor = vec4(prefilterRes, 1.0f);
}