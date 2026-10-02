#pragma once

#include "base.h"

inline constexpr size_t MAXIMUM_ALIGNMENT = 256;

struct mem_arena
{
    u8    *Base;
    size_t Size;      // reserved address space: the hard limit
    size_t Committed; // bytes backed by real memory; == Size for InitArena arenas
    size_t Used;
    s32    TempCount;
};

struct mem_temp
{
    mem_arena *Arena;
    size_t     Used;
};

// OS virtual memory: reserve address space up front, commit pages as they are used.
void *OsReserve(size_t Size);
[[nodiscard]] b32 OsCommit(void *Memory, size_t Size);
void OsRelease(void *Memory, size_t Size);

// Reserves Size bytes of address space. Nothing is committed until PushSize needs it.
[[nodiscard]] b32 CreateArena(size_t Size, mem_arena *Arena);
// Only for arenas from CreateArena: InitArena arenas do not own their memory.
void DestroyArena(mem_arena *Arena);
// Wraps memory someone else owns and has already committed (e.g. a mapped GPU buffer).
void InitArena(void *Base, size_t Size, mem_arena *Arena);

void *PushSize(mem_arena *Arena, size_t Size, size_t Alignment = MAXIMUM_ALIGNMENT);
void ResetArena(mem_arena *Arena);

mem_temp BeginTempMemory(mem_arena *Arena);
void EndTempMemory(mem_temp Temp);  // roll back everything pushed since Begin
void KeepTempMemory(mem_temp Temp); // close the region but keep what was pushed

mem_temp GetScratch(mem_arena **Conflicts = nullptr, int ConflictCount = 0);

#ifdef JAUTILS_ARENA_IMPLEMENTATION
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <sys/mman.h>
#else
#error "jaUtils: unsupported platform"
#endif

// Commit in chunks so PushSize does not call the OS for every small push.
// A multiple of the 4 KB page size; 64 KB is also Windows' reservation granularity.
static constexpr size_t COMMIT_GRANULARITY = KB(64);

void *
OsReserve(size_t Size)
{
#if defined(_WIN32)
    return VirtualAlloc(nullptr, Size, MEM_RESERVE, PAGE_NOACCESS);
#else
    void *Result = mmap(nullptr, Size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    return (Result != MAP_FAILED) ? Result : nullptr;
#endif
}

b32
OsCommit(void *Memory, size_t Size)
{
#if defined(_WIN32)
    return VirtualAlloc(Memory, Size, MEM_COMMIT, PAGE_READWRITE) ? 1 : 0;
#else
    return (mprotect(Memory, Size, PROT_READ | PROT_WRITE) == 0) ? 1 : 0;
#endif
}

void
OsRelease(void *Memory, size_t Size)
{
#if defined(_WIN32)
    (void)Size;
    VirtualFree(Memory, 0, MEM_RELEASE);
#else
    munmap(Memory, Size);
#endif
}

b32
CreateArena(size_t Size, mem_arena *Arena)
{
    *Arena       = {};
    void *Memory = OsReserve(Size);
    if(!Memory)
    {
        return 0;
    }
    Arena->Base = static_cast<u8 *>(Memory);
    Arena->Size = Size;
    return 1;
}

void
DestroyArena(mem_arena *Arena)
{
    JA_ASSERT(Arena->TempCount == 0);
    if(Arena->Base)
    {
        OsRelease(Arena->Base, Arena->Size);
    }
    *Arena = {};
}

void
InitArena(void *Base, size_t Size, mem_arena *Arena)
{
    *Arena = {.Base = static_cast<u8 *>(Base), .Size = Size, .Committed = Size, .Used = 0, .TempCount = 0};
}

static size_t
AlignmentPadding(umm Address, size_t Alignment)
{
    umm Mask = Alignment - 1;
    return static_cast<size_t>((Alignment - (Address & Mask)) & Mask);
}

// Grow the committed region so [0, End) is usable. Never shrinks.
static b32
EnsureCommitted(mem_arena *Arena, size_t End)
{
    if(End <= Arena->Committed)
    {
        return 1;
    }

    size_t NewCommitted = (End + COMMIT_GRANULARITY - 1) & ~(COMMIT_GRANULARITY - 1);
    if(NewCommitted > Arena->Size)
    {
        NewCommitted = Arena->Size;
    }
    if(!OsCommit(Arena->Base + Arena->Committed, NewCommitted - Arena->Committed))
    {
        return 0;
    }
    Arena->Committed = NewCommitted;
    return 1;
}

void *
PushSize(mem_arena *Arena, size_t Size, size_t Alignment)
{
    JA_ASSERT(Arena);
    JA_ASSERT(Alignment != 0 && (Alignment & (Alignment - 1)) == 0);
    size_t Padding   = AlignmentPadding(reinterpret_cast<umm>(Arena->Base) + Arena->Used, Alignment);
    size_t Remaining = Arena->Size - Arena->Used;

    if(!(Padding <= Remaining && Size <= Remaining - Padding))
    {
        JA_ASSERT(!"arena full");
        return nullptr;
    }

    size_t End = Arena->Used + Padding + Size;
    if(!EnsureCommitted(Arena, End))
    {
        return nullptr; // out of physical memory / page file: a runtime failure, not a bug
    }

    u8 *Result  = Arena->Base + Arena->Used + Padding;
    Arena->Used = End;
    return Result;
}

void
ResetArena(mem_arena *Arena)
{
    JA_ASSERT(Arena->TempCount == 0);
    Arena->Used = 0; // committed pages stay committed for the next use
}

mem_temp
BeginTempMemory(mem_arena *Arena)
{
    JA_ASSERT(Arena);
    ++Arena->TempCount;
    return {.Arena = Arena, .Used = Arena->Used};
}

void
EndTempMemory(mem_temp Temp)
{
    mem_arena *Arena = Temp.Arena;
    JA_ASSERT(Arena);
    JA_ASSERT(Arena->TempCount > 0);
    JA_ASSERT(Arena->Used >= Temp.Used);

    Arena->Used = Temp.Used;
    --Arena->TempCount;
}

void
KeepTempMemory(mem_temp Temp)
{
    mem_arena *Arena = Temp.Arena;
    JA_ASSERT(Arena);
    JA_ASSERT(Arena->TempCount > 0);
    JA_ASSERT(Arena->Used >= Temp.Used);

    --Arena->TempCount;
}

static constexpr u32    SCRATCH_COUNT    = 2;
static constexpr size_t MAX_SCRATCH_SIZE = MB(64);

static thread_local mem_arena GlobalScratch[SCRATCH_COUNT];

mem_temp
GetScratch(mem_arena **Conflicts, int ConflictCount)
{
    for(u32 Index = 0; Index < SCRATCH_COUNT; ++Index)
    {
        mem_arena *Scratch = &GlobalScratch[Index];

        b32 IsConflict = 0;
        for(int Conflict = 0; Conflict < ConflictCount; ++Conflict)
        {
            if(Conflicts[Conflict] == Scratch)
            {
                IsConflict = 1;
                break;
            }
        }
        if(IsConflict)
        {
            continue;
        }

        if(!Scratch->Base && !CreateArena(MAX_SCRATCH_SIZE, Scratch))
        {
            JA_ASSERT(!"reserving scratch arena failed");
            return {};
        }
        return BeginTempMemory(Scratch);
    }
    JA_ASSERT(!"all scratch arenas conflicted");
    return {};
}

#endif
