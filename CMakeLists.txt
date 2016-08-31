CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Project
project(quaviss)
SET (VERSION_MAJOR 1)
SET (VERSION_MINOR 0)
SET (VERSION_PATCH 0)
SET (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Environment
include_directories (${PROJECT_SOURCE_DIR}/include)
link_directories (${PROJECT_SOURCE_DIR}/lib)

# Subfolders
add_subdirectory (bin)
add_subdirectory (doc)
add_subdirectory (test)
