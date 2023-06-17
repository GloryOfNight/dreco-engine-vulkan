#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outUVW;

layout(set = 0, binding = 0) uniform readonly Camera 
{
    mat4 view;
    mat4 proj;
} cameraData;

void main() {
    // make sure skybox doesn't get cut by projection near/far plane
    gl_Position = cameraData.proj * mat4(mat3(cameraData.view)) * vec4(inPosition * 10, 0.1);

    outUVW = inPosition;
}