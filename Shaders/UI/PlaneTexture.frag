#version 450

//Inputs from vertex shader
layout(location = 0) in vec2 in_texcoord;

//Output to the attachment provided by framebuffer
layout(location = 0) out vec4 outColor;

//Bindings for these must be provided in the descriptor sets and bound to the pipeline object
layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main()
{
    outColor = texture(texSampler, in_texcoord);
}