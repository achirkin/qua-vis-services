#version 450

layout(location = 0) in vec4 vCartesianPosition;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec4 fColor;

void main() {
  fColor = vec4(vCartesianPosition[2], vColor[0], 1, 1);
}
