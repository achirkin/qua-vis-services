#version 450

layout(location = 0) in vec4 vCartesianPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in float rmax;

layout(location = 0) out vec4 fColor;

void main() {
  // if the image should be displayed, vColor needs to be changed in the vertex!!
  fColor = vec4(length(vCartesianPosition)/rmax, 1, 1, 1);
}
