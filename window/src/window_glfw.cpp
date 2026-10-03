#include "jawindow/window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>

static void
ProcessButton(ja_button *Button, b32 IsDown)
{
    if(Button->EndedDown != IsDown)
    {
        Button->EndedDown = IsDown;
        ++Button->HalfTransitionCount;
    }
}

static void
KeyCallback(GLFWwindow *Handle, int Key, int /*Scancode*/, int Action, int /*Mods*/)
{
    if(Action == GLFW_REPEAT)
    {
        return;
    }

    ja_input *Input = static_cast<ja_input *>(glfwGetWindowUserPointer(Handle));
    ja_key    Mapped;
    switch(Key)
    {
        case GLFW_KEY_W:
            Mapped = JA_KEY_W;
            break;
        case GLFW_KEY_A:
            Mapped = JA_KEY_A;
            break;
        case GLFW_KEY_S:
            Mapped = JA_KEY_S;
            break;
        case GLFW_KEY_D:
            Mapped = JA_KEY_D;
            break;
        case GLFW_KEY_ESCAPE:
            Mapped = JA_KEY_ESCAPE;
            break;
        default:
            return;
    }
    ProcessButton(&Input->Keys[Mapped], (Action == GLFW_PRESS) ? 1 : 0);
}

b32
OpenWindow(int Width, int Height, const char *Title, ja_window *Window)
{
    *Window = {};
    if(!glfwInit())
    {
        fprintf(stderr, "jawindow: glfwInit failed\n");
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    Window->Handle = glfwCreateWindow(Width, Height, Title, nullptr, nullptr);
    if(!Window->Handle)
    {
        fprintf(stderr, "jawindow: glfwCreateWindow failed\n");
        glfwTerminate();
        return 0;
    }

    glfwSetKeyCallback(Window->Handle, KeyCallback);

    glfwMakeContextCurrent(Window->Handle);
    glfwSwapInterval(1);
    return 1;
}

void
CloseWindow(ja_window *Window)
{
    if(Window->Handle)
    {
        glfwDestroyWindow(Window->Handle);
    }
    glfwTerminate(); // safe even if glfwInit never succeeded
    *Window = {};
}

void
PresentWindow(ja_window *Window)
{
    glfwSwapBuffers(Window->Handle);
}

void
GetWindowFramebufferSize(ja_window *Window, int *Width, int *Height)
{
    glfwGetFramebufferSize(Window->Handle, Width, Height);
}

f64
GetTimeSeconds()
{
    return glfwGetTime();
}

ja_gl_get_proc
GetWindowProcLoader()
{
    return glfwGetProcAddress;
}

b32
WindowShouldClose(ja_window *Window)
{
    return glfwWindowShouldClose(Window->Handle);
}

void
PumpEvents(ja_window *Window, ja_input *Input)
{
    for(u32 Index = 0; Index < JA_KEY_COUNT; ++Index)
    {
        Input->Keys[Index].HalfTransitionCount = 0;
    }

    glfwSetWindowUserPointer(Window->Handle, Input);
    glfwPollEvents();
}
