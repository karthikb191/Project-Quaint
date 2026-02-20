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
    vec3 forward = normalize(direction);
    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(up, forward);
    up = cross(forward, right);

    const float PI = 3.14159265f;
    const float TWOPI = 3.14159265f * 2.0f;
    const float PIBy2 = 3.14159265f * 0.5f;
    const float azimuthSamples = 100;
    const float zenithSamples = 100;

    const float dPhi = TWOPI / azimuthSamples;
    const float dTheta = PIBy2 / zenithSamples;

    //Check the notes for explanation on how this works
    // phi = azimuth angle. along the horizontal circumference of sphere
    // theta = zenith angle. Starts from the top of sphere. We calculate till Pi/2, which represents the hemisphere
    
    vec3 irradiance = vec3(0.0f);
    float numSamples = 0;
    for(float phi = 0; phi < TWOPI; phi += dPhi)
    {
        for(float theta = 0; theta < PIBy2; theta += dTheta)
        {
            //We first transform polar coordinates to cartesian coordinates. These coordinates would be in tangent space
            float z = cos(theta);
            float r = sin(theta);
            float x = r * cos(phi);
            float y = r * sin(phi);
            vec3 tangentVec = vec3(x, y, z);

            // This tangentVec is a vector with right, up and forward as it's basis vectors.
            // So, to convert it to world space, we just multiply and accumulate the basis vectors with the respective coordinates of tangent vector
            // Hemisphere is oriented around the normal vector. So, zenith value(z) is mapped to forward.
            // Then x is mapped to right and y to up
            vec3 wsVec = right * tangentVec.x + up * tangentVec.y + forward * tangentVec.z;

            irradiance += texture(envMap, wsVec).xyz * cos(theta) * sin(theta);

            ++numSamples;
        }
    }

    //Check notes on why we are multiplying with PI now!!
    irradiance = PI * irradiance * (1.0f / numSamples);
    return irradiance;
}

void main()
{
    vec3 irradianceRes = sampleIrradiance(in_fragLocalPos.xyz);
    outColor = vec4(irradianceRes, 1.0f);
}