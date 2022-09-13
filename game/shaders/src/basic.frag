#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler[4];
layout(set = 1, binding = 1) uniform materialUniform
{
    uint baseColorIndex;
    vec4 baseColorFactor;

    uint metallicRoughnessIndex;
    float metallicFactor;
    float roughnessFactor;

    uint emissiveIndex;
    vec3 emissiveFactor;
	
    uint normalIndex;   
    float normalScale;
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