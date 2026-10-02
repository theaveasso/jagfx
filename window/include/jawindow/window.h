#pragma once

#include "jautils/base.h"
#include "jawindow/input.h"

struct GLFWwindow;
struct ja_window
{
    GLFWwindow *Handle;
};

typedef void (*ja_gl_proc)(void);
typedef ja_gl_proc (*ja_gl_get_proc)(const char *Name);

[[nodiscard]] b32
OpenWindow(int Width, int Height, const char *Title, ja_window *Window);
void CloseWindow(ja_window *Window); // safe on a zeroed or failed-to-open window
void PresentWindow(ja_window *Window);
// In pixels, which can differ from the window size on high-DPI screens.
void GetWindowFramebufferSize(ja_window *Window, int *Width, int *Height);

[[nodiscard]] ja_gl_get_proc GetWindowProcLoader();

[[nodiscard]] b32 WindowShouldClose(ja_window *Window);
void PumpEvents(ja_window *Window, ja_input *Input);
