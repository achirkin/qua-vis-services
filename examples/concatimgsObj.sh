#!/bin/bash

PARAMS="-d 1 -a 0.2 -r 5.0"

mkdir -p images
cat <<EOT > ../src/shaders/shader.vert
#version 450

// I assume metric distance.
// Distance from human rotation axis to the eyes.
#define FORWARD_D 0.15
// Half of the distance between eyes.
#define SIDE_D 0.034

layout(binding = 0) uniform UniformBufferObject {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vCartesianPosition;
layout(location = 1) out vec3 vColor;

void main() {
  // Compute vector from observer to vertex
  vCartesianPosition = inPosition - ubo.observation_point;
  float r2 = dot(vCartesianPosition,vCartesianPosition),
        scale = length(vCartesianPosition.xy) / r2;
  vec3  displacement = - cross(vec3(0,0, scale * SIDE_D), vCartesianPosition)
                       - (scale * FORWARD_D) * vec3(vCartesianPosition.x, vCartesianPosition.y, 0.0);
  vCartesianPosition = (r2 > dot(displacement,displacement))
                     ? vCartesianPosition + displacement
                     : 0.00001 * vCartesianPosition;
  vColor = inColor;
}
EOT
pushd ../
make
popd
../bin/quavis-generic-service $PARAMS -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/mooctask.geojson" < objObservePoint.txt
mv images images-left


mkdir -p images
cat <<EOT > ../src/shaders/shader.vert
#version 450

// I assume metric distance.
// Distance from human rotation axis to the eyes.
#define FORWARD_D 0.15
// Half of the distance between eyes.
#define SIDE_D 0.034


layout(binding = 0) uniform UniformBufferObject {
  vec3 observation_point;
  float r_max;
  float alpha_max;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 vCartesianPosition;
layout(location = 1) out vec3 vColor;

void main() {
  // Compute vector from observer to vertex
  vCartesianPosition = inPosition - ubo.observation_point;
  float r2 = dot(vCartesianPosition,vCartesianPosition),
        scale = length(vCartesianPosition.xy) / r2;
  vec3  displacement = + cross(vec3(0,0, scale * SIDE_D), vCartesianPosition)
                       - (scale * FORWARD_D) * vec3(vCartesianPosition.x, vCartesianPosition.y, 0.0);
  vCartesianPosition = (r2 > dot(displacement,displacement))
                     ? vCartesianPosition + displacement
                     : 0.00001 * vCartesianPosition;
  vColor = inColor;
}
EOT
pushd ../
make
popd
../bin/quavis-generic-service $PARAMS -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/mooctask.geojson" < objObservePoint.txt
mv images images-right

mkdir -p images-result
for imgname in images-left/*
do
  convert -append images-right/${imgname##*/} images-left/${imgname##*/} images-result/${imgname##*/}
done
rm -rf images-left images-right


../bin/quavis-generic-service -l 1 -a 0.05 -r 5.0 -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/mooctask.geojson" < objObservePoint.txt

