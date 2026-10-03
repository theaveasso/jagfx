add_executable(jagfx_cube main.cpp)

target_link_libraries(jagfx_cube PRIVATE jaGFX::jaGFX jaGFX::jaWindow)
target_compile_definitions(
    jagfx_cube
    PRIVATE BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)
