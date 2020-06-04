#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
vec4 gl_Position;                                                                                                                     
};

layout(location = 0) out vec3 fragColor;

// layout(location = 1) in mat4 matrix;
mat4 matrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
    );

vec2 positions[3] = vec2[](
vec2(0.0, -0.5), 
vec2(0.5, 0.5),
vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = matrix * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}