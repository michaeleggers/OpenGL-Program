#version 460 core

#extension GL_ARB_gpu_shader_int64: enable

layout(std140, binding = 0) uniform PerFrameData {
    uniform mat4 view;
    uniform mat4 projection;
    uniform vec3 viewPos;
};

struct Vertex {
    float pos[3];
    float uv[2];
    float normal[3];
    float tangent[3];
};

struct Index {
    uint idx;
};

struct DrawData {
    uint indexOffset;
    uint vertexOffset;
    uint materialID;
    uint transformID;
};

layout(std430, binding = 1) readonly buffer Vertices {
    Vertex in_Vertices[];
};

layout(std430, binding = 2) readonly buffer Indices {
    Index in_Indices[];
};

layout(std430, binding = 3) readonly buffer DrawDatas {
    DrawData in_DrawData[];
};


layout(location = 0) out vec3               out_color;
layout(location = 1) out flat uint          out_materialID;
layout(location = 2) out vec2               out_uv;
layout(location = 3) out vec3               out_normal;
layout(location = 4) out vec3               out_position;
layout(location = 5) out mat3               out_TBNmat;

vec3 getPosition(uint i) {
    return vec3(in_Vertices[i].pos[0], in_Vertices[i].pos[1], in_Vertices[i].pos[2]);
}

vec2 getUV(uint i) {
    return vec2(in_Vertices[i].uv[0], in_Vertices[i].uv[1]);
}

vec3 getNormal(uint i) {
    return vec3(in_Vertices[i].normal[0], in_Vertices[i].normal[1], in_Vertices[i].normal[2]);
}

vec3 getTangent(uint i) {
    return vec3(in_Vertices[i].tangent[0], in_Vertices[i].tangent[1], in_Vertices[i].tangent[2]);
}

void main()
{
    DrawData dd = in_DrawData[gl_BaseInstance];
    
    uint vertexIndex = in_Indices[dd.indexOffset + gl_VertexID].idx;

    // uint vertexIndex = in_Indices[gl_VertexID].idx;
    // uint vertexIndex = gl_InstanceID + gl_VertexID;

    // uint vertexIndex = gl_VertexID;
    vec3 worldPos = getPosition(vertexIndex);

    vec4 pos = projection * view * vec4(worldPos, 1.0);
    vec2 uv = getUV(vertexIndex);
    vec4 normal = vec4(getNormal(vertexIndex), 0.0);
    vec3 tangent = normalize(getTangent(vertexIndex));
    vec3 bitangent = normalize(cross(tangent, normal.xyz));
    mat3 TBNmat = mat3(tangent, bitangent, normal);

    gl_Position = pos;
    out_color = vec3(uv, 0.2);
    out_color = 0.5*normal.xyz + 0.5;
    out_materialID = dd.materialID;
    out_uv         = uv;
    out_normal     = normal.xyz;
    out_TBNmat     = TBNmat;
    out_position   = worldPos;
}
