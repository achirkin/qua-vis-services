1. Compile compute shader to SPIR-V
```
glslangValidator -V shaders/shader.area.comp -o shaders/shader.area.comp.spv
glslangValidator -V shaders/shader.2.area.comp -o shaders/shader.2.area.comp.spv
```

2. Run analysis
```
../bin/quavis-generic-service -a 0.1 -r 1000000.0 -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/empower-shack.geojson" < data/empower-shack-grid.txt
```

 * `a` is the maxium angle in radians
 * `r` is the maximum distance
 * 
