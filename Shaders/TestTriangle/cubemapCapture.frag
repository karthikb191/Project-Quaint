#version 450

// Inputs from vertex shader
layout(location = 0) in vec4 in_fragLocalPos;
layout(location = 1) in vec2 in_normal;

// Uniforms from host
layout(binding = 1) uniform sampler2D envMap;

// Output interface
layout(location = 0) out vec4 outColor;


//TODO: Comment the explanation here
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    // Negating y because 1 points upwards, whose asin value is pi/2
    // on dividing by pi, it's value becomes 0.5
    // The code before was adding 0.5, which would effectively make it point to the bottom of the texture
    // Negating v.y will fix that issue
    // On reading more on it, it looks like opengl uses bottom-left as texture origin.
    // Vulkan uses top-left for it's texture origin
    vec2 uv = vec2(atan(v.z, v.x), asin(-v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(in_fragLocalPos.xyz));
    vec3 color = texture(envMap, uv).rgb;
    outColor = vec4(color, 1.0f);
}