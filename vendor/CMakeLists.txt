set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_library(khronos INTERFACE)
target_include_directories(
    khronos
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/khronos/include
)
add_library(jaGFX::Khronos ALIAS khronos)
