cmake_minimum_required(VERSION 3.16)
project(raytracer LANGUAGES CXX)
find_package(PNG REQUIRED)

add_library(raytracer
    src/vector.cpp src/ray.cpp src/sphere.cpp src/triangle.cpp
    src/intersection.cpp src/geometry.cpp src/scene.cpp src/raytracer.cpp)
target_compile_features(raytracer PUBLIC cxx_std_20)
target_include_directories(raytracer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(raytracer PUBLIC PNG::PNG)
set_target_properties(raytracer PROPERTIES CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(raytracer PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(render_scene src/main.cpp)
target_link_libraries(render_scene PRIVATE raytracer)
