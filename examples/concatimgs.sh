#!/bin/bash



mkdir -p images
cat <<EOT > ../src/shaders/shader.vert
#version 450

// I assume metric distance.
// Distance from human rotation axis to the eyes.
#define FORWARD_D 0.8
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
  float r = length(vCartesianPosition);
  vCartesianPosition = (r*r > FORWARD_D*FORWARD_D + SIDE_D*SIDE_D) ? ((1 - FORWARD_D / r ) * vCartesianPosition + cross(vec3(0,0,SIDE_D/r), vCartesianPosition)) : 0.00001 * vCartesianPosition;
  vColor = inColor;
}
EOT
pushd ../
make
popd
../bin/quavis-generic-service -d 1 -a 0.2 -r 150.0 -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/mooctask.geojson" < data/mooktask_grid.txt
mv images images-left


mkdir -p images
cat <<EOT > ../src/shaders/shader.vert
#version 450

// I assume metric distance.
// Distance from human rotation axis to the eyes.
#define FORWARD_D 0.8
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
  float r = length(vCartesianPosition);
  vCartesianPosition = (r*r > FORWARD_D*FORWARD_D + SIDE_D*SIDE_D) ? ((1 - FORWARD_D / r ) * vCartesianPosition - cross(vec3(0,0,SIDE_D/r), vCartesianPosition)) : 0.00001 * vCartesianPosition;
  vColor = inColor;
}
EOT
pushd ../
make
popd
../bin/quavis-generic-service -d 1 -a 0.2 -r 150.0 -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/mooctask.geojson" < data/mooktask_grid.txt
mv images images-right

mkdir -p images-result
for imgname in images-left/*
do
  convert -append images-left/${imgname##*/} images-right/${imgname##*/} images-result/${imgname##*/}
done
rm -rf images-left images-right
