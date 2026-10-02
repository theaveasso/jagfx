#pragma once

#include "arena.h"
#include <stdio.h>

struct file
{
    void  *Data;
    size_t Size;
};

[[nodiscard]] file ReadEntireFile(const char *Path, mem_arena *Arena);
[[nodiscard]] b32 FileExists(const char *Path);

#ifdef JAUTILS_FILE_IMPLEMENTATION

file
ReadEntireFile(const char *Path, mem_arena *Arena)
{
    FILE *File = fopen(Path, "rb");
    if(!File)
    {
        return {};
    }

    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    if(FileSize <= 0)
    {
        fclose(File);
        return {};
    }

    size_t   Size = static_cast<size_t>(FileSize);
    mem_temp Mark = BeginTempMemory(Arena);

    u8 *Data = static_cast<u8 *>(PushSize(Arena, Size + 1, 1));
    if(!Data)
    {
        EndTempMemory(Mark);
        fclose(File);
        return {};
    }

    size_t Read = fread(Data, 1, Size, File);
    fclose(File);
    if(Read != Size)
    {
        EndTempMemory(Mark);
        return {};
    }

    Data[Size] = '\0';
    KeepTempMemory(Mark);
    return {.Data = Data, .Size = Size};
}

b32
FileExists(const char *Path)
{
    FILE *File = fopen(Path, "rb");
    if(File)
    {
        fclose(File);
    }
    return File != nullptr;
}
#endif
