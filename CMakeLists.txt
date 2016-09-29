CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# CMAKE
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Project
project(quaviss)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)
set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Requirements
find_package (Threads REQUIRED)
find_package (Vulkan REQUIRED)

# Headers
include_directories (${PROJECT_SOURCE_DIR}/include)
include_directories (${VULKAN_INCLUDE_DIR})

# Linker
link_directories (${PROJECT_SOURCE_DIR}/lib)
link_directories (${VULKAN_LIBRARY})

set (ENV{VK_LAYER_PATH} "/usr/local/x86_64/etc/explicit_layer.d/")

## Threads
if(THREADS_HAVE_PTHREAD_ARG)
  compile_options(PUBLIC"-pthread")
endif()

if(CMAKE_THREAD_LIBS_INIT)
  link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif()

# Compiler
if (WIN32)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
else (WIN32)
    add_definitions (-std=c++14)
    add_definitions (-pipe)
    add_definitions ("-Wall -O3")
endif (WIN32)

# Configure
configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/config/quavis/version.h.in"
  "${CMAKE_SOURCE_DIR}/include/quavis/version.h"
)

# Shaders
add_custom_target(shaders)
if (WIN32)
  # TODO: Compile shaders in CMake under windows
else (WIN32)
  file(GLOB files "${CMAKE_SOURCE_DIR}/src/shaders/shader.*")
  add_custom_command(TARGET shaders PRE_BUILD COMMAND echo "" > include/quavis/shaders.h)
  foreach(file ${files})
    string(REGEX REPLACE "^[^\\/]*[\\|/]" "" filename ${file})
    set(filepath "src/shaders/${filename}")
    add_custom_command(TARGET shaders PRE_BUILD COMMAND glslangValidator -V ${filepath} -o ${filepath}.spv)
    add_custom_command(TARGET shaders PRE_BUILD COMMAND xxd -i ${filepath}.spv >> include/quavis/shaders.h)
    add_custom_command(TARGET shaders POST_BUILD COMMAND rm ${filepath}.spv)
  endforeach()
endif ()

# Quavis
add_library(quavis SHARED
  "${CMAKE_SOURCE_DIR}/src/vk/instance.cc"
  "${CMAKE_SOURCE_DIR}/src/vk/device/physicaldevice.cc"
  "${CMAKE_SOURCE_DIR}/src/vk/device/logicaldevice.cc"
  "${CMAKE_SOURCE_DIR}/src/vk/memory/allocator.cc"
)
target_link_libraries(quavis ${VULKAN_LIBRARY})
add_dependencies(quavis shaders)

# Tests
add_executable (testvulkan
  "${CMAKE_SOURCE_DIR}/test/vulkan2.cc"
)
target_link_libraries (testvulkan quavis)
add_dependencies(testvulkan quavis)
