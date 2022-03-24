#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[4];
layout(binding = 2) uniform materialUniform
{
    uint baseColorIndex;
    uint metallicRoughnessIndex;
    uint normalIndex;
    uint emissiveIndex;
    // 16 ^ group all by 16
    vec4 baseColorFactor;
    // 16 ^
    vec3 emissiveFactor;
    float normalScale;
    // 16 ^
    float metallicFactor;
    float roughnessFactor;
    // 8 ^
} mat;

void main() {
    vec4 baseColor = mat.baseColorFactor + inColor.rgba;
    if (mat.baseColorIndex != -1)
    {
        baseColor = texture(texSampler[mat.baseColorIndex], inUV) * mat.baseColorFactor;
    }
    if (mat.emissiveIndex != -1)
    {
        baseColor += texture(texSampler[mat.emissiveIndex], inUV) * vec4(mat.emissiveFactor, 1);
    }

    outColor = baseColor;
}