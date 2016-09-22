# qua-vis-services

* libvulkan-dev

# Shader compilation
For shader compilation, install the [GLSL Reference Compiler](https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/) and run:

```
sh src/shaders/compile.sh
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
