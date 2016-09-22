#version 450
#extension GL_ARB_separate_shader_objects : enable

in vec3 tePosition;
in vec3 tePatchDistance;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
