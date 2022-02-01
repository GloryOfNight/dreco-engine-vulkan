#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

layout(binding = 0) uniform Camera 
{
    mat4 viewProj;
} cameraData;

layout( push_constant ) uniform constants
{
    mat4 model;
} modelData;

void main() {
    outNormal = inNormal;
    outUV = inUV;
    gl_Position = cameraData.viewProj * modelData.model * vec4(inPosition, 1.0);
}