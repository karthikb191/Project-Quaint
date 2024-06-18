#version 450

//Vertex Input Bindings for this shader might not be needed.
layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_texcoord;

//Outputs to fragment shader
layout(location = 0) out vec2 out_texcoord;

void main()
{
    out_texcoord = in_texcoord;
}