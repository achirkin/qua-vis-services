# qua-vis-services

* libvulkan-dev

# Shader compilation
For shader compilation, use the [GLSL Reference Compiler](https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/):

```
glslangValidator -V src/shaders/shader.vert -o src/shaders/vert.spv
glslangValidator -V src/shaders/shader.frag -o src/shaders/frag.spv
```

Afterwards, create a header file to include it in the shared library:

```
xxd -i src/shaders/vert.spv > include/quavis/shaders.h
xxd -i src/shaders/frag.spv >> include/quavis/shaders.h
```
