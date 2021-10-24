#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    outNormal = inNormal;
    outUV = inUV;
    gl_Position = ubo.proj * ubo.view * ubo.model  * vec4(inPosition, 1.0);
}