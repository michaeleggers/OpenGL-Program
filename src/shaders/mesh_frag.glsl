#version 460 core

#extension GL_ARB_bindless_texture : require

#define M_PI 3.1415926535897932384626433832795

layout(std140, binding = 0) uniform PerFrameData {
    uniform mat4 view;
    uniform mat4 projection;
    uniform vec3 viewPos;
};

struct Material {
    uint  albedoTextureID;
    uint  opacityTextureID;
    uint  metalnessTextureID;
    uint  roughnessTextureID;
    uint  normalTextureID;
    uint  hasAlbedo;
    uint  hasOpacity;
    uint  hasMetalness;
    uint  hasRoughness;
    uint  hasNormal;
    float opacity;
};

layout (location = 0) in vec3               color;
layout (location = 1) in flat uint          materialID;
layout (location = 2) in vec2               uv;
layout (location = 3) in vec3               normal;
layout (location = 4) in vec3               position;
layout (location = 5) in mat3               TBNmat;

layout(std430, binding = 4) readonly buffer Materials {
    Material in_Materials[];
};

layout(std430, binding = 5) readonly buffer Textures {
    uvec2 in_Samplers[32];
};

// Texture block
//layout (binding = 1, std140) uniform TEXTURE_BLOCK
//{
//    sampler2D      in_Samplers[32];
//};

//layout(binding = 5) uniform sampler2D textures[];

out vec4 outColor;

vec3 lights[19] = vec3[19](
    vec3(1000, 0, 2000),
    vec3(-1000, 0, 1000),
    vec3(1000, 0, -1000),
    vec3(1000, 0, -1000),
    vec3(1000, 0, -1000),
    vec3(-1000, 0, 1000),
    vec3(-1000, 0, 1000),
    vec3(0, 4000, -100),
    vec3(0, 400, -100),
    vec3(100, 400, -100),
    vec3(100, 0, -100),
    vec3(100, 0, -100),
    vec3(100, -400, 200),
    vec3(100, -400, 200),
    vec3(100, -400, 200),
    vec3(0, -400, 0),
    vec3(0, 0, 400),
    vec3(0, 0, 400),
    vec3(0, 0, 400)
);

float distributionGGX(vec3 N, vec3 H, float a) {
    float aSquared = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotHSquared = NdotH*NdotH;
    float aSquaredMin1 = aSquared - 1;
    float denomTemp = NdotHSquared * aSquaredMin1 + 1;
    float denom = M_PI * denomTemp * denomTemp;

    return aSquared / denom;
}

float schlickGGX(vec3 N, vec3 V, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float oneMinK = 1 - k;

    return NdotV / (NdotV*oneMinK + k);
}

// F0: surface reflection at zero incidence (=> how much does the surface
//     reflects when looking directly at it.)
vec3 fresenelSchlick(vec3 H, vec3 V, vec3 F0) {
    return F0 + (vec3(1.0)-F0)* pow( 1.0-max(dot(H, V), 0.0), 5.0 );
}

void main()
{
    vec3 lightColor = vec3(150000.0, 150000.0, 150000.0);

    // Get the material info
    Material material = in_Materials[materialID];

    // Standard values if no texture is set
    vec4 albedoColor = vec4(1, 0, 0, 1);
    vec4 opacityMap   = vec4(1, 1, 1, 1);
    vec3  metalness = vec3(1.0);
    float roughness = 1.0; 
    vec3 perFragmentNormal = normal;
    // Sample values from textures if available
    if (material.hasAlbedo == 1) {
        //sampler2D albedoSampler = sampler2D(material.albedoTextureID);
        albedoColor = texture(sampler2D(in_Samplers[material.albedoTextureID]), uv);
    }
    if (material.hasOpacity == 1) {
        //sampler2D opacitySampler = sampler2D(material.opacityTextureID);
        opacityMap = texture(sampler2D(in_Samplers[material.opacityTextureID]), uv);
    }
    if (material.hasMetalness == 1) {
        metalness = texture(sampler2D(in_Samplers[material.metalnessTextureID]), uv).bbb;
    }
    if (material.hasRoughness == 1) {
        roughness = texture(sampler2D(in_Samplers[material.roughnessTextureID]), uv).g;
    }
    if (material.hasNormal == 1) {
        perFragmentNormal = texture(sampler2D(in_Samplers[material.normalTextureID]), uv).xyz;
        perFragmentNormal = normalize((2.0*perFragmentNormal - 1.0)); // map from [0.0, 1.0] -> [-1.0, 1.0]

        // convert normal from tangent-space to worldspace
        perFragmentNormal = TBNmat * perFragmentNormal;
    }

    // Convert albedo from sRGB to linear space
    albedoColor.rgb = pow(albedoColor.rgb, vec3(2.2));

    // Compute some values used later
    vec3 viewDir = normalize(viewPos - position); // vector from fragment to viewer
    
    // Compute Radiance to viewer
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColor.rgb, metalness);
    vec3 outgoingRadiance = vec3(0.0);
    for (int i = 0; i < 19; i++) {
        vec3  lightPos      = lights[i];
        vec3  lightDir      = normalize(lightPos - position);
        float lightDistance = length(lightPos - position);
        vec3  halfwayVec    = (viewDir + lightDir) / (length(viewDir + lightDir));
        float attenuation   = 1.0 / (lightDistance*lightDistance);
        vec3  radiance      = lightColor * attenuation;
        float D = distributionGGX(perFragmentNormal, halfwayVec, roughness);
        float G = schlickGGX(perFragmentNormal, viewDir, k) * schlickGGX(normal, lightDir, k);
        vec3  F = fresenelSchlick(halfwayVec, viewDir, F0);
        vec3  cookTorrace = D*G*F / 4*( max(dot(viewDir, perFragmentNormal), 0.0) * max(dot(lightDir, perFragmentNormal), 0.0) + 0.0001 );
        float cosTheta = max(0.0, dot(lightDir, perFragmentNormal));
        vec3 kS = F0;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness.b;
        outgoingRadiance +=  ((kD*albedoColor.rgb/M_PI) + cookTorrace) * radiance * cosTheta;
    }


    //outColor = vec4(pow(outgoingRadiance, vec3(1.0/2.2)), 1.0);

    // exposure tone mapping
    float exposure = 1.0;
    outgoingRadiance = vec3(1.0) - exp(-outgoingRadiance * exposure);
    outgoingRadiance = outgoingRadiance / (outgoingRadiance + vec3(1.0));
    
    // Imporvised ambient term
    vec3 ambient = vec3(0.4) * albedoColor.rgb; // * ambientOcclusion
    outgoingRadiance += ambient;
    
    // gamma correction
    outColor = vec4(pow(outgoingRadiance, vec3(1.0/2.2)), 1.0); 

    // albedoColor.a = material.opacity * albedoColor.a;
    //outColor = vec4(outgoingRadiance, 1.0);
    //outColor = vec4(color, 1.0);

    //outColor = albedoColor;
    //outColor = vec4(perFragmentNormal*0.5 + 0.5, 1.0);
    //outColor = vec4(vec3(roughness), 1.0);
    //outColor = vec4(metalness, 1.0);
    

}
