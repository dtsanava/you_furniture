cmake_minimum_required(VERSION 3.22)
project(m3d_android CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(M3D_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../core ${CMAKE_CURRENT_BINARY_DIR}/core)

add_library(m3d SHARED
    native_bridge.cpp
    renderer.cpp
)
target_compile_options(m3d PRIVATE -Wall -Wextra)
target_link_libraries(m3d PRIVATE m3d_core GLESv3 log)
