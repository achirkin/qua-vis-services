#version 450

layout(location = 0) in vec3 gCartesianPosition;
layout(location = 1) in vec4 gSphericalPosition;
layout(location = 2) in vec3 gColor;

layout(location = 0) out vec4 fColor;

void main() {
  fColor = vec4(gSphericalPosition[2], gSphericalPosition[2], gSphericalPosition[2], 1.0);
}
