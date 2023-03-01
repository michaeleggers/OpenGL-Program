#version 460 core

#extension GL_ARB_bindless_texture: enable


#define M_PI 3.1415926535897932384626433832795

struct Material {
    uvec2 diffuseTextureID;
    uvec2 opacityTextureID;
    uint  hasDiffuse;
    uint  hasOpacity;
    float opacity;
};

layout (location = 0) in vec3               color;
layout (location = 1) in flat uint          materialID;
layout (location = 2) in vec2               uv;
layout (location = 3) in vec3               normal;
layout (location = 4) in vec3               position;

layout(std430, binding = 4) readonly buffer Materials {
    Material in_Materials[];
};

layout(binding = 5) uniform sampler2D textures[];

out vec4 outColor;

vec3 lights[15] = vec3[15](
    vec3(100, 10, 20),
    vec3(-100, 10, 10),
    vec3(0, 400, 100),
    vec3(0, 400, 100),
    vec3(0, 400, 100),
    vec3(0, 400, 100),
    vec3(0, 400, 100),
    vec3(0, 400, -100),
    vec3(0, 400, -100),
    vec3(100, 400, -100),
    vec3(100, 400, -100),
    vec3(100, 400, -100),
    vec3(100, -400, 200),
    vec3(100, -400, 200),
    vec3(100, -400, 200)
);

void main()
{
    vec3 pointLightPos = vec3(10, 10, 10);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    Material material = in_Materials[materialID];
    vec4 diffuseColor = vec4(1, 0, 0, 1);
    vec4 opacityMap   = vec4(1, 1, 1, 1);
    if (material.hasDiffuse == 1) {
        sampler2D diffuseSampler = sampler2D(material.diffuseTextureID);
        diffuseColor = texture(diffuseSampler, uv);
    }
    if (material.hasOpacity == 1) {
        sampler2D opacitySampler = sampler2D(material.opacityTextureID);
        opacityMap = texture(opacitySampler, uv);
    }

    vec3 outgoingRadiance = vec3(0);
    for (int i = 0; i < 15; i++) {
        vec3 currentLightPos = lights[i];
        vec3 lightDir = normalize(currentLightPos - position);
        float cosTheta = max(0.0, dot(lightDir, normalize(normal)));
        outgoingRadiance += lightColor * (diffuseColor.rgb/M_PI) * cosTheta;
    }

    outColor = vec4(color, 1.0);
    // diffuseColor.a = material.opacity * diffuseColor.a;
    outColor = vec4(outgoingRadiance, material.opacity);
    // outColor = diffuseColor;

}
