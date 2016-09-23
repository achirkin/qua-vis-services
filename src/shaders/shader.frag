#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 gCartesianPosition;
layout(location = 1) in vec4 gSphericalPosition;
layout(location = 2) in vec3 gColor;

layout(location = 0) out vec4 fColor;

void main() {
    fColor = vec4(gColor, gSphericalPosition[2]);
}
