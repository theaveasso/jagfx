#include "shader_expand.h"
#include "jautils/file.h"

#include <stdio.h>
#include <string.h>

constexpr u32 MAX_INCLUDE_DEPTH = 8;

struct shader_source
{
    mem_arena       *Out;
    mem_arena       *Files;
    expanded_shader *Result;
};

static b32
Emit(shader_source *Source, const char *Text, size_t Size)
{
    char *Dest = (char *)PushSize(Source->Out, Size, 1);
    if(!Dest)
    {
        fprintf(stderr, "jagfx: shader source too large\n");
        return 0;
    }
    memcpy(Dest, Text, Size);
    return 1;
}

static b32
EmitLine(shader_source *Source, u32 Line, u32 FileIndex)
{
    char Buffer[32];
    int  Length = snprintf(Buffer, sizeof(Buffer), "#line %u %u\n", Line, FileIndex);
    return Emit(Source, Buffer, (size_t)Length);
}

static b32
ExpandFile(shader_source *Source, const char *Path, u32 Depth)
{
    expanded_shader *Result = Source->Result;
    if(Depth > MAX_INCLUDE_DEPTH)
    {
        fprintf(stderr, "jagfx: %s includes nested deeper than %u (cycle?)\n", Path, MAX_INCLUDE_DEPTH);
        return 0;
    }
    if(Result->FileCount == MAX_SHADER_FILES)
    {
        fprintf(stderr, "jagfx: %s more than %u shader files\n", Path, MAX_SHADER_FILES);
        return 0;
    }

    file File = ReadEntireFile(Path, Source->Files);
    if(!File.Data)
    {
        fprintf(stderr, "jagfx: failed to read %s\n", Path);
        return 0;
    }

    u32 FileIndex                = Result->FileCount++;
    Result->FileNames[FileIndex] = PushString(Source->Files, Path);

    // #version must be the first line the driver sees
    if(Depth > 0 && !EmitLine(Source, 1, FileIndex))
    {
        return 0;
    }

    const char *At   = (const char *)File.Data;
    u32         Line = 1;
    while(*At)
    {
        const char *End = At;
        while(*End && *End != '\n')
        {
            ++End;
        }
        const char *Next = *End ? End + 1 : End;

        if(strncmp(At, "#include", 8) == 0)
        {
            const char *P = At + 8;
            while(*P == ' ' || *P == '\t')
            {
                ++P;
            }
            if(*P == '<')
            {
                if(!Emit(Source, "\n", 1))
                {
                    return 0;
                }
                At = Next;
                ++Line;
                continue;
            }
            if(*P != '"')
            {
                fprintf(stderr, "jagfx: %s(%u): expected \"file\" or <file> after #include\n", Path, Line);
                return 0;
            }
            ++P;

            const char *NameStart = P;
            while(P < End && *P != '"')
            {
                ++P;
            }
            size_t NameLength = (size_t)(P - NameStart);
            if(P == End || NameLength == 0 || NameLength >= 256)
            {
                fprintf(stderr, "jagfx: %s(%u) malformed #include\n", Path, Line);
                return 0;
            }
            char Name[256];
            memcpy(Name, NameStart, NameLength);
            Name[NameLength] = '\0';

            int FolderLength = static_cast<int>(PathFolderLength(Path));

            char Candidate[512];
            int  Length = snprintf(Candidate, sizeof(Candidate), "%.*s%s", FolderLength, Path, Name);
            if(!FileExists(Candidate))
            {
                Length = snprintf(Candidate, sizeof(Candidate), "%s%s", JAGFX_SHADER_INCLUDE_DIR, Name);
            }
            if(Length < 0 || static_cast<size_t>(Length) >= sizeof(Candidate))
            {
                fprintf(stderr, "jagfx: %s(%u): include path too long\n", Path, Line);
                return 0;
            }

            if(!ExpandFile(Source, Candidate, Depth + 1))
            {
                return 0;
            }

            if(!EmitLine(Source, Line + 1, FileIndex))
            {
                return 0;
            }
        }
        else if(!Emit(Source, At, size_t(Next - At)))
        {
            return 0;
        }
        At = Next;
        ++Line;
    }

    return Emit(Source, "\n", 1);
}

b32
ExpandShader(const char *Path, mem_arena *Out, mem_arena *Files, expanded_shader *Result)
{
    *Result = {};
    JA_ASSERT(Out != Files && "expanded text must stay contiguous in Out");

    shader_source Source = {.Out = Out, .Files = Files, .Result = Result};
    const char   *Text   = (const char *)Out->Base + Out->Used;
    if(!ExpandFile(&Source, Path, 0) || !Emit(&Source, "", 1))
    {
        return 0;
    }
    Result->Text = Text;
    return 1;
}
