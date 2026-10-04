#pragma once

#include "jautils/base.h"

enum ja_key
{
    JA_KEY_A,
    JA_KEY_B,
    JA_KEY_C,
    JA_KEY_D,
    JA_KEY_E,
    JA_KEY_F,
    JA_KEY_G,
    JA_KEY_H,
    JA_KEY_I,
    JA_KEY_J,
    JA_KEY_K,
    JA_KEY_L,
    JA_KEY_M,
    JA_KEY_N,
    JA_KEY_O,
    JA_KEY_P,
    JA_KEY_Q,
    JA_KEY_R,
    JA_KEY_S,
    JA_KEY_T,
    JA_KEY_U,
    JA_KEY_V,
    JA_KEY_W,
    JA_KEY_X,
    JA_KEY_Y,
    JA_KEY_Z,

    JA_KEY_0,
    JA_KEY_1,
    JA_KEY_2,
    JA_KEY_3,
    JA_KEY_4,
    JA_KEY_5,
    JA_KEY_6,
    JA_KEY_7,
    JA_KEY_8,
    JA_KEY_9,

    JA_KEY_F1,
    JA_KEY_F2,
    JA_KEY_F3,
    JA_KEY_F4,
    JA_KEY_F5,
    JA_KEY_F6,
    JA_KEY_F7,
    JA_KEY_F8,
    JA_KEY_F9,
    JA_KEY_F10,
    JA_KEY_F11,
    JA_KEY_F12,

    JA_KEY_UP,
    JA_KEY_DOWN,
    JA_KEY_LEFT,
    JA_KEY_RIGHT,

    JA_KEY_SPACE,
    JA_KEY_ENTER,
    JA_KEY_ESCAPE,
    JA_KEY_TAB,
    JA_KEY_BACKSPACE,
    JA_KEY_GRAVE,

    JA_KEY_INSERT,
    JA_KEY_DELETE,
    JA_KEY_HOME,
    JA_KEY_END,
    JA_KEY_PAGE_UP,
    JA_KEY_PAGE_DOWN,

    JA_KEY_LEFT_SHIFT,
    JA_KEY_RIGHT_SHIFT,
    JA_KEY_LEFT_CONTROL,
    JA_KEY_RIGHT_CONTROL,
    JA_KEY_LEFT_ALT,
    JA_KEY_RIGHT_ALT,

    JA_KEY_COUNT,
};

struct ja_button
{
    s32 HalfTransitionCount;
    b32 EndedDown;
};

struct ja_input
{
    ja_button Keys[JA_KEY_COUNT];
};

inline b32
IsDown(ja_button Button)
{
    return Button.EndedDown;
}

inline b32
WasPressed(ja_button Button)
{
    return Button.HalfTransitionCount > 1 || (Button.HalfTransitionCount == 1 && Button.EndedDown) ? 1 : 0;
}
