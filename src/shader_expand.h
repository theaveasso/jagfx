#pragma once

#include "jautils/arena.h"
#include "jautils/base.h"

constexpr u32 MAX_SHADER_FILES = 16;

struct expanded_shader
{
    const char *Text;
    u32         FileCount;
    const char *FileNames[MAX_SHADER_FILES];
};

[[nodiscard]] b32 ExpandShader(const char *Path, mem_arena *Out, mem_arena *Files, expanded_shader *Result);
