cmake_minimum_required(VERSION 3.22)
project(m3d_core CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-independent editor core: geometry, camera, scene. No Android or GL code here.
add_library(m3d_core STATIC
    src/math.cpp
    src/camera.cpp
    src/mesh.cpp
)
target_include_directories(m3d_core PUBLIC include)
if(MSVC)
    target_compile_options(m3d_core PRIVATE /W4)
else()
    target_compile_options(m3d_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(M3D_BUILD_TESTS "Build core tests" ON)
if(M3D_BUILD_TESTS AND NOT ANDROID)
    enable_testing()
    add_executable(m3d_core_tests tests/core_tests.cpp)
    target_link_libraries(m3d_core_tests PRIVATE m3d_core)
    add_test(NAME m3d_core_tests COMMAND m3d_core_tests)
endif()
