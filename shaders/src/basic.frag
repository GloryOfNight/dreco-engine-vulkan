#version 450

layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outColor;

struct pbr_metallic_roughness
{
    vec4 baseColorFactor;

    double metallicFactor;

    double roughnessFactor;
};

layout(binding = 1) uniform sampler2D texSampler[4]; // 0 - baseColor, 1 - metallicRoughness, 2 - normal, 3 - emissive, 
layout(binding = 2) uniform MaterialUniformObject {
    uint normalTextureIndex;

    uint emissiveTextureIndex;

    pbr_metallic_roughness pbrMetallicRoughness;
} material;

void main() {
    outColor = texture(texSampler[0], inUV);
}