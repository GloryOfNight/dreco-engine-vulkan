#version 450

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube cubemap;

void main()
{
	outColor = texture(cubemap, inUVW);
}