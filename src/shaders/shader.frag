#version 450

layout(location = 0) in vec3 gCartesianPosition;
layout(location = 1) in vec4 gSphericalPosition;
layout(location = 2) in vec3 gNormal;

layout(location = 0) out vec4 vColor;

void main() {
  vec3 sun = normalize(vec3(3.0f,4.0f,5.0f)),
       nor = sign(dot(gNormal, - gCartesianPosition)) * normalize(gNormal);
  float str = dot(sun, nor),
        val = clamp(0.6f + 0.5f*str - 0.7f*gSphericalPosition[2], 0.0f, 1.0f);
  //vColor = vec4( val, 1.0f, 1.0f, 1.0f); //  + dot(sun, gColor) - gSphericalPosition[2]
  vColor = vec4(gSphericalPosition[2],1.0f,1.0f,1.0f);
}
