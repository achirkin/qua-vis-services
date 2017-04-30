1. Compile compute shader to SPIR-V
```
glslangValidator -V shader.area.comp -o shader.area.comp.spv
glslangValidator -V shader.2.area.comp -o shader.2.area.comp.spv
```

2. Run analysis
```
../bin/quavis-generic-service -s "shader.area.comp.spv" -p "shader.2.area.comp.spv" -f "empower-shack.geojson" -a 0.1 -r 1000000.0 < points.txt
```
