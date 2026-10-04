#include "jawindow/input.h"
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

static_assert(JA_KEY_Z - JA_KEY_A == GLFW_KEY_Z - GLFW_KEY_A);
static_assert(JA_KEY_9 - JA_KEY_0 == GLFW_KEY_9 - GLFW_KEY_0);
static_assert(JA_KEY_F12 - JA_KEY_F1 == GLFW_KEY_F12 - GLFW_KEY_F1);

[[nodiscard]] static b32
MapKey(int Key, ja_key *Mapped)
{
    if(Key >= GLFW_KEY_A && Key <= GLFW_KEY_Z)
    {
        *Mapped = (ja_key)(JA_KEY_A + (Key - GLFW_KEY_A));
        return 1;
    }
    if(Key >= GLFW_KEY_0 && Key <= GLFW_KEY_9)
    {
        *Mapped = (ja_key)(JA_KEY_0 + (Key - GLFW_KEY_0));
        return 1;
    }
    if(Key >= GLFW_KEY_F1 && Key <= GLFW_KEY_F12)
    {
        *Mapped = (ja_key)(JA_KEY_F1 + (Key - GLFW_KEY_F1));
        return 1;
    }

    switch(Key)
    {
        case GLFW_KEY_UP:
            *Mapped = JA_KEY_UP;
            return 1;
        case GLFW_KEY_DOWN:
            *Mapped = JA_KEY_DOWN;
            return 1;
        case GLFW_KEY_LEFT:
            *Mapped = JA_KEY_LEFT;
            return 1;
        case GLFW_KEY_RIGHT:
            *Mapped = JA_KEY_RIGHT;
            return 1;
        case GLFW_KEY_SPACE:
            *Mapped = JA_KEY_SPACE;
            return 1;
        case GLFW_KEY_ENTER:
            *Mapped = JA_KEY_ENTER;
            return 1;
        case GLFW_KEY_ESCAPE:
            *Mapped = JA_KEY_ESCAPE;
            return 1;
        case GLFW_KEY_TAB:
            *Mapped = JA_KEY_TAB;
            return 1;
        case GLFW_KEY_BACKSPACE:
            *Mapped = JA_KEY_BACKSPACE;
            return 1;
        case GLFW_KEY_GRAVE_ACCENT:
            *Mapped = JA_KEY_GRAVE;
            return 1;
        case GLFW_KEY_INSERT:
            *Mapped = JA_KEY_INSERT;
            return 1;
        case GLFW_KEY_DELETE:
            *Mapped = JA_KEY_DELETE;
            return 1;
        case GLFW_KEY_HOME:
            *Mapped = JA_KEY_HOME;
            return 1;
        case GLFW_KEY_END:
            *Mapped = JA_KEY_END;
            return 1;
        case GLFW_KEY_PAGE_UP:
            *Mapped = JA_KEY_PAGE_UP;
            return 1;
        case GLFW_KEY_PAGE_DOWN:
            *Mapped = JA_KEY_PAGE_DOWN;
            return 1;
        case GLFW_KEY_LEFT_SHIFT:
            *Mapped = JA_KEY_LEFT_SHIFT;
            return 1;
        case GLFW_KEY_RIGHT_SHIFT:
            *Mapped = JA_KEY_RIGHT_SHIFT;
            return 1;
        case GLFW_KEY_LEFT_CONTROL:
            *Mapped = JA_KEY_LEFT_CONTROL;
            return 1;
        case GLFW_KEY_RIGHT_CONTROL:
            *Mapped = JA_KEY_RIGHT_CONTROL;
            return 1;
        case GLFW_KEY_LEFT_ALT:
            *Mapped = JA_KEY_LEFT_ALT;
            return 1;
        case GLFW_KEY_RIGHT_ALT:
            *Mapped = JA_KEY_RIGHT_ALT;
            return 1;
        default:
            return 0;
    }
}

static void
KeyCallback(GLFWwindow *Handle, int Key, int /*Scancode*/, int Action, int /*Mods*/)
{
    if(Action == GLFW_REPEAT)
    {
        return;
    }

    ja_key Mapped;
    if(!MapKey(Key, &Mapped))
    {
        return;
    }

    ja_input *Input = static_cast<ja_input *>(glfwGetWindowUserPointer(Handle));
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
    glfwTerminate();
    *Window = {};
}

void
PresentWindow(ja_window *Window)
{
    glfwSwapBuffers(Window->Handle);
}

static void
BeginEvents(ja_window *Window, ja_input *Input)
{
    for(u32 Index = 0; Index < JA_KEY_COUNT; ++Index)
    {
        Input->Keys[Index].HalfTransitionCount = 0;
    }
    glfwSetWindowUserPointer(Window->Handle, Input);
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
WaitEvents(ja_window *Window, ja_input *Input)
{
    BeginEvents(Window, Input);
    glfwWaitEvents();
}

void
PumpEvents(ja_window *Window, ja_input *Input)
{
    BeginEvents(Window, Input);
    glfwPollEvents();
}
