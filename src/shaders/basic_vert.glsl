#version 460 core

vec3 verts[3] = vec3[3](
    vec3(-1, -1, 1),
    vec3(1, -1, 1),
    vec3(0.0, 1, 1)
);

void main()
{
    gl_Position = vec4(verts[gl_VertexID], 1);
}
