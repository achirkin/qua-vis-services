# qua-vis-services

# Requirements

* libvulkan-dev
* libasio-dev
* [GLSL Reference Compiler](https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/)

# Compiling

The project can be compiled on both Linux using CMake and the respective build system such as make

1. Clone the project using the recursive tag: `git clone --recursive https://github.com/Kelinago/qua-vis-services`
2. Build the project using CMake and Make: `cmake . && make`
3. After running helen or Luci, run the service using `bin/service`

# Using validation layers

* Download the [VulkanSDK](https://lunarg.com/vulkan-sdk/)
* **Temporary**: Extract the files and copy them into a system directory
  ```
    sudo cp -r VulkanSDK/*/x86_64 /usr/local/x86_64
  ```
* Run test files with
  ```
    VK_LAYER_PATH=/usr/local/x86_64/etc/explicit_layer.d/ bin/testvulkan
  ```

# Running on a descrete nvidia gpu with nvidia-prime

I use wayland and integrated intel GPU on my ubuntu laptop. To make nvidia GPU compute something in such a setting, I need to do couple extra steps in bash *before* running the service:
```bash
sudo modprobe nvidia
export LD_LIBRARY_PATH=/usr/lib/nvidia-384:$LD_LIBRARY_PATH
unset DISPLAY
unset unset XDG_SESSION_TYPE
```
The commands above set up an environment to run vulkan using an nvidia GPU headless (without a display environment). 
