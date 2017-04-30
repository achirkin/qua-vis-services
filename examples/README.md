# Usage
1. Compile compute shader to SPIR-V using the Khronos [GlslangValidator](https://cvs.khronos.org/svn/repos/ogl/trunk/ecosystem/public/sdk/tools/glslang/Install/)
2. Run the generic analysis `quavis-generic-service` where
 * `-a` is the maxium angle in radians
 * `-r` is the maximum visible distance
 * `-s` is the first-stage compute shader (aggregating row-wise)
 * `-t` is the second-stage compute shader (aggregating column-wise)
 * `-f` is the geojson file to be analysid
 * *stdin* is the list of observation points in the format
```
x1 y1 z1
x2 y2 z2
...
```

# Example
```
cd examples
glslangValidator -V shaders/shader.area.comp -o shaders/shader.area.comp.spv
glslangValidator -V shaders/shader.2.area.comp -o shaders/shader.2.area.comp.spv
../bin/quavis-generic-service -a 0.1 -r 1000000.0 -s "shaders/shader.area.comp.spv" -t "shaders/shader.2.area.comp.spv" -f "data/empower-shack.geojson" < data/empower-shack-grid.txt
```
