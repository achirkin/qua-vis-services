CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# CMAKE
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Project
project(quaviss)
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
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
link_libraries (${VULKAN_LIBRARY})

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
endif (WIN32)

# Build
find_package(glfw3 3.2 REQUIRED)
add_executable (testvulkan "${CMAKE_SOURCE_DIR}/test/vulkan.cc")
target_link_libraries (testvulkan glfw)
add_definitions (-DDEBUG -g)
