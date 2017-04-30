# qua-vis

# Compiling

1. Install nvidia-drivers and vulkan:
`sudo apt-get install libvulkan-dev`
2. Install glslangValidator
```
wget https://cvs.khronos.org/svn/repos/ogl/trunk/ecosystem/public/sdk/tools/glslang/Install/Linux/glslangValidator
sudo cp glslangValidator /usr/local/bin
sudo +x /usr/local/bin/glslangValidator
```

The project can be compiled on both Linux using CMake and the respective build system such as make
1. Clone the project using the recursive tag: `git clone --recursive git@github.com:mtfranzen/qua-vis-services.git`
2. Switch to this branch `git checkout standalone`
3. Build the project using CMake and Make: `cmake . && make`
4. Continue with the [examples](examples/README.md)
