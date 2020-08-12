#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 p;
layout(location = 1) in vec3 c;

layout(set = 0, binding = 0) uniform global_uniform {
    mat4 vp;
} global;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
    gl_Position =  global.vp * vec4(p, 1.0);
    fragColor = c;
}