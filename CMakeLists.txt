cmake_minimum_required(VERSION 3.28)

project(jaGFX VERSION 0.1.0 LANGUAGES CXX)

option(jaGFX_BUILD_WINDOW "Build jaGFX window" ON)
option(jaGFX_BUILD_EXAMPLES "Build jaGFX examples" ON)
option(jaGFX_BUILD_TESTS "Build jaGFX tests" OFF)

find_package(OpenGL REQUIRED)

add_subdirectory(vendor)
add_subdirectory(utility)

add_library(jaGFX STATIC include/jagfx/jagfx.h src/jagfx.cpp src/shader_expand.h src/shader_expand.cpp)

add_library(jaGFX::jaGFX ALIAS jaGFX)

target_include_directories(jaGFX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(jaGFX PUBLIC jaGFX::jaUtils PRIVATE jaGFX::Khronos)

target_compile_definitions(
    jaGFX
    PRIVATE JAGFX_SHADER_INCLUDE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

if(jaGFX_BUILD_WINDOW)
    add_subdirectory(window)
endif()

if(jaGFX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
