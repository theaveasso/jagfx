#pragma once

#include "jautils/base.h"

enum ja_key
{
    JA_KEY_W,
    JA_KEY_A,
    JA_KEY_S,
    JA_KEY_D,
    JA_KEY_ESCAPE,
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
