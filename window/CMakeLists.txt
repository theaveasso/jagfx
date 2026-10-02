add_library(jaWindow STATIC
  include/jawindow/input.h
  include/jawindow/window.h
  src/window_glfw.cpp)

add_library(jaGFX::jaWindow ALIAS jaWindow)

target_include_directories(jaWindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# window.h includes jautils -> PUBLIC. Only window_glfw.cpp includes GLFW -> PRIVATE.
target_link_libraries(jaWindow PUBLIC jaGFX::jaUtils PRIVATE glfw)
