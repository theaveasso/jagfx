add_library(
    jaUtils
    STATIC
    include/jautils/arena.h
    include/jautils/base.h
    include/jautils/file.h
    include/jautils/math.h
    src/jautils.cpp
)

add_library(jaGFX::jaUtils ALIAS jaUtils)

target_include_directories(jaUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
