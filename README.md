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

# Using validation layers

* Download the [VulkanSDK](https://lunarg.com/vulkan-sdk/)
* **Temporary**: Extract the files and copy them into a system directory:
  ```
    sudo cp -r VulkanSDK/*/x86_64 /usr/local/x86_64
  ```
* Run test files with
  ```
    VK_LAYER_PATH=/usr/local/x86_64/etc/explicit_layer.d/ bin/testvulkan
  ```
