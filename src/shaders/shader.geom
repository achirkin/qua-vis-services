#version 450

layout(triangles) in;
layout(location = 0) in vec3 teCartesianPosition[3];
layout(location = 1) in vec4 teSphericalPosition[3];
layout(location = 2) in vec3 teColor[3];

layout(triangle_strip, max_vertices = 3) out;
layout(location = 0) out vec3 gCartesianPosition;
layout(location = 1) out vec4 gSphericalPosition;
layout(location = 2) out vec3 gColor;

void EmitCorrectedPrimitive() {
  gCartesianPosition = teCartesianPosition[0];
  gSphericalPosition = teSphericalPosition[0];
  //gSphericalPosition[0] -= 2 * int(teSphericalPosition[0][0] >= 1);
  gColor = teColor[0];
  gl_Position = gSphericalPosition;
  EmitVertex();

  gCartesianPosition = teCartesianPosition[1];
  gSphericalPosition = teSphericalPosition[1];
  //gSphericalPosition[0] -= 2 * int(teSphericalPosition[1][0] >= 1);
  gColor = teColor[1];
  gl_Position = gSphericalPosition;
  EmitVertex();

  gCartesianPosition = teCartesianPosition[2];
  gSphericalPosition = teSphericalPosition[2];
  //gSphericalPosition[0] -= 2 * int(teSphericalPosition[2][0] >= 1);
  gColor = teColor[2];
  gl_Position = gSphericalPosition;
  EmitVertex();

  EndPrimitive();
}

void main() {
  EmitCorrectedPrimitive();
}
