#version 450

const vec2 quadVertices[6] = 
{
    vec2(-1.0f, -1.0f),
    vec2(-1.0f, 1.0f),
    vec2(1.0f, 1.0f),
    vec2(-1.0f, -1.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, -1.0f)
};

const vec2 texcoord[6] = 
{
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 1),
    vec2(1, 0)
};

layout(location = 0) out vec2 fragWorldPos;

void main()
{
    vec2 vert = quadVertices[gl_VertexIndex];
    vec2 tex = texcoord[gl_VertexIndex];
    
    gl_Position = vec4(vert, 0.0, 1.0);
    fragWorldPos = vec2(tex);
}