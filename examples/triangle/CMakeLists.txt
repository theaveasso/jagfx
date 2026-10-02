add_executable(jagfx_triangle main.cpp)

target_link_libraries(jagfx_triangle PRIVATE jaGFX::jaGFX jaGFX::jaWindow)
target_compile_definitions(jagfx_triangle PRIVATE SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
