#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 tePosition;
layout(location = 1) in vec3 teColor;
layout(location = 2) in vec3 tePatchDistance;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(teColor, tePosition[3]);
}
