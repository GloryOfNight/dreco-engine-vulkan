#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D baseColor;
layout(set = 1, binding = 1) uniform sampler2D metallicRoughness;
layout(set = 1, binding = 2) uniform sampler2D emissive;
layout(set = 1, binding = 3) uniform sampler2D normal;

layout(set = 1, binding = 4) uniform readonly materialUniform
{
    vec4 baseColorFactor;
    vec3 emissiveFactor;

    bool hasBaseColor;
    bool hasEmissiveIndex;
    bool hasMetallicRoughness;
    bool hasNormalIndex; 

    float metallicFactor;
    float roughnessFactor;
    float normalScale;
} mat;

void main() {
    vec4 outColorTemp = mat.baseColorFactor + inColor.rgba;
    if (mat.hasBaseColor)
    {
        outColorTemp = texture(baseColor, inUV) * mat.baseColorFactor;
    }
    if (mat.hasEmissiveIndex)
    {
        outColorTemp += texture(emissive, inUV) * vec4(mat.emissiveFactor, 1);
    }

    outColor = outColorTemp;
}