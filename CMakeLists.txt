CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# CMAKE
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Project
project(quavis-services)
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
    add_definitions ("-Wall")
    add_definitions('-g')
    add_definitions(-DELPP_THREAD_SAFE)
    add_definitions(-DELPP_DISABLE_DEFAULT_CRASH_HANDLING)
    add_definitions(-DASIO_STANDALONE)
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
  file(GLOB files "${CMAKE_SOURCE_DIR}/src/shaders/shader*")
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
  "${CMAKE_SOURCE_DIR}/src/context.cc"
)

target_link_libraries(quavis ${VULKAN_LIBRARY})
add_dependencies(quavis shaders)

# Services
add_executable (quavis-isovist-area
  "${CMAKE_SOURCE_DIR}/src/area-service.cc"
)
target_link_libraries (quavis-isovist-area quavis)
target_link_libraries (quavis-isovist-area luciconnect)
add_dependencies(quavis-isovist-area quavis)
add_dependencies(quavis-isovist-area luciconnect)

add_executable (quavis-isovist-minradial
  "${CMAKE_SOURCE_DIR}/src/minradial-service.cc"
)
target_link_libraries (quavis-isovist-minradial quavis)
target_link_libraries (quavis-isovist-minradial luciconnect)
add_dependencies(quavis-isovist-minradial quavis)
add_dependencies(quavis-isovist-minradial luciconnect)

add_executable (quavis-isovist-maxradial
  "${CMAKE_SOURCE_DIR}/src/maxradial-service.cc"
)
target_link_libraries (quavis-isovist-maxradial quavis)
target_link_libraries (quavis-isovist-maxradial luciconnect)
add_dependencies(quavis-isovist-maxradial quavis)
add_dependencies(quavis-isovist-maxradial luciconnect)


# Install Directivey
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/quavis DESTINATION include COMPONENT headers)
install(TARGETS quavis DESTINATION lib COMPONENT libraries)
install(TARGETS quavis-isovist-area DESTINATION bin COMPONENT binaries)
install(TARGETS quavis-isovist-minradial DESTINATION bin COMPONENT binaries)
install(TARGETS quavis-isovist-maxradial DESTINATION bin COMPONENT binaries)

# Packaging
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_CONTACT "franzen@arch.ethz.ch")
set (CPACK_PACKAGE_MAINTAINER "Michael Franzen <franzen@arch.ethz.ch>")
set (CPACK_PACKAGE_NAME "quavis-services")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Isovist Services for QUA-Server" )

## Components
set (CPACK_COMPONENTS_ALL libraries binaries headers)

set (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Library")
set (CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Shared Libraries")
set (CPACK_COMPONENT_LIBRARIES_REQUIRED luciconnect)

set (CPACK_COMPONENT_BINARIES_DISPLAY_NAME "Binaries")
set (CPACK_COMPONENT_BINARIES_DESCRIPTION "The service binaries")
set (CPACK_COMPONENT_BINARIES_REQUIRED libraries)

set (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
set (CPACK_COMPONENT_HEADERS_DESCRIPTION "C++ Headers for Quavis-Services")
set (CPACK_COMPONENT_HEADERS_DEPENDS libraries binaries)

# Component settings
set (CPACK_COMPONENT_LIBRARIES_GROUP "Development")
set (CPACK_COMPONENT_BINARIES_GROUP "Development")
set (CPACK_COMPONENT_HEADERS_GROUP "Development")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION "${VERSION}")

if (WIN32)
  set (CPACK_GENERATOR "NSIS")
else (WIN32)
  set (CPACK_GENERATOR "DEB")
endif (WIN32)

include (CPack)
