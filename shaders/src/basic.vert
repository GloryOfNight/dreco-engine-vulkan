#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec4 outColor;

layout(set = 0, binding = 0) uniform Camera 
{
    mat4 view;
    mat4 viewProj;
} cameraData;

layout( push_constant ) uniform constants
{
    mat4 model;
} modelData;

void main() {
    outUV = inUV;
    outColor = inColor;
    outNormal = mat3(cameraData.view * modelData.model) * inNormal;

    gl_Position = cameraData.viewProj * modelData.model * vec4(inPosition, 1.0);
}