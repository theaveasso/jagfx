add_executable(jagfx_deferred_renderer main.cpp)

target_link_libraries(
    jagfx_deferred_renderer
    PRIVATE jaGFX::jaGFX jaGFX::jaWindow
)
target_compile_definitions(
    jagfx_deferred_renderer
    PRIVATE BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)
