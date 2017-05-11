#!/bin/bash
for i in `seq 1 100`; do
  ../bin/quavis-generic-service -s shaders/shader.area.comp.spv -t shaders/shader.2.area.comp.spv -a 0.1 -r 10000 -u 1 -f data/mooctask.geojson < data/mooctask-grid-big.txt >> results.txt
done
