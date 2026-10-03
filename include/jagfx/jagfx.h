#pragma once

#include "jautils/arena.h"
#include "jautils/base.h"
#include "jautils/math.h"
#include "jagfx/jagfx_shared.h"

constexpr u32 FRAMES_IN_FLIGHT = 2;

typedef void (*gl_proc)(void);
typedef gl_proc (*gl_get_proc)(const char *Name);

enum sampler_filter : u32
{
    SAMPLER_FILTER_LINEAR = 0,
    SAMPLER_FILTER_NEAREST,
};

enum sampler_wrap : u32
{
    SAMPLER_WRAP_REPEAT = 0,
    SAMPLER_WRAP_CLAMP,
};

enum cull_mode : u32
{
    CULL_NONE = 0,
    CULL_BACK,
    CULL_FRONT,
};

enum blend_mode : u32
{
    BLEND_NONE = 0,
    BLEND_ALPHA,    // Src * SrcAlpha + Dst * (1 - SrcAlpha)
    BLEND_ADDITIVE, // Src + Dst
};

enum barrier_flags : u32
{
    BARRIER_STORAGE  = 1 << 0,
    BARRIER_INDIRECT = 1 << 1,
};

// Zero means off for every field, like a zeroed Vulkan create-info.
struct graphics_pipeline_desc
{
    const char *VertexPath;
    const char *FragmentPath;
    b32         DepthTest;
    b32         DepthWrite;
    cull_mode   Cull;
    blend_mode  Blend;
};

struct compute_pipeline_desc
{
    const char *ComputePath;
};

// Raster state is baked in at creation and applied by every draw, so nothing leaks between draws.
struct pipeline
{
    u32        Program;
    b32        DepthTest;
    b32        DepthWrite;
    cull_mode  Cull;
    blend_mode Blend;
    u32        GroupSize[3];
};

struct texture
{
    u32 Handle;
    s32 Width;
    s32 Height;
    u64 Bindless;
};

struct sampler_desc
{
    sampler_filter Filter;
    sampler_wrap   Wrap;
    f32            MaxAnisotropy;
};

struct sampler
{
    u32 Handle;
};

template <typename T>
struct gpu_cpu_range
{
    T         *Cpu;
    gpu_ptr<T> Gpu;
    u32        Count;
};

struct gpu_heap
{
    u32       Buffer;
    mem_arena Arena;
};

struct frame_slot
{
    void     *Fence; // a GLsync, opaque out here
    mem_arena Arena;
};

struct frame_ring
{
    frame_slot  Slots[FRAMES_IN_FLIGHT];
    u32         FrameIndex;
    frame_slot *Current;
};

[[nodiscard]] b32 JagfxInit(gl_get_proc GetProc);
void JagfxShutdown();

[[nodiscard]] b32 CreateGraphicsPipeline(const graphics_pipeline_desc *Desc, pipeline *Pipeline);
[[nodiscard]] b32 CreateComputeProgram(const compute_pipeline_desc *Desc, pipeline *Pipeline);
void DestroyPipeline(pipeline *Pipeline);

[[nodiscard]] b32 CreateGpuHeap(size_t Size, gpu_heap *Heap);
void DestroyGpuHeap(gpu_heap *Heap);

[[nodiscard]] b32 CreateFrameRing(gpu_heap *Heap, size_t Size, frame_ring *Ring);
void DestroyFrameRing(frame_ring *Ring);
[[nodiscard]] frame_slot *BeginFrame(frame_ring *Ring);
void EndFrame(frame_ring *Ring);

[[nodiscard]] b32 CreateSampler(const sampler_desc *Desc, sampler *Sampler);
void DestroySampler(sampler *Sampler);

[[nodiscard]] b32 LoadTexture2D(const char *Path, s32 Width, s32 Height, const sampler *Sampler, texture *Texture);
[[nodiscard]] b32 CreateTexture2D(s32 Width, s32 Height, const void *RGBA8Pixels, const sampler *Sampler, texture *Texture);
void DestroyTexture(texture *Texture);

void BeginRendering(s32 Width, s32 Height, vec4 ClearColor);
void EndRendering();

void Barrier(u32 Flags);
void DrawArraysInstancedUntyped(pipeline *Pipeline, u32 RootOffset, u32 VertexCount, u32 InstanceCount);
void DrawElementsInstancedUntyped(pipeline *Pipeline, u32 RootOffset, u32 IndexOffset, u32 IndexCount, u32 InstanceCount);
void DrawIndexedIndirectUntyped(pipeline *Pipeline, u32 RootOffset, u32 CommandOffset, u32 DrawCount);
void DispatchGroupsUntyped(pipeline *Pipeline, u32 RootOffset, u32 GroupsX, u32 GroupsY = 1, u32 GroupsZ = 1);

template <typename T>
void
DrawArraysInstanced(pipeline *Pipeline, gpu_ptr<T> Root, u32 VertexCount, u32 InstanceCount = 1)
{
    DrawArraysInstancedUntyped(Pipeline, Root.Offset, VertexCount, InstanceCount);
}

template <typename T>
void
DrawIndexed(pipeline *Pipeline, gpu_ptr<T> Root, gpu_ptr<u16> IndexOffset, u32 IndexCount, u32 InstanceCount = 1)
{
    DrawElementsInstancedUntyped(Pipeline, Root.Offset, IndexOffset.Offset, IndexCount, InstanceCount);
}

template <typename T>
void
DrawIndexedIndirect(pipeline *Pipeline, gpu_ptr<T> Root, gpu_ptr<draw_indexed_command> Commands, u32 DrawCount = 1)
{
    DrawIndexedIndirectUntyped(Pipeline, Root.Offset, Commands.Offset, DrawCount);
}

template <typename T>
void
Dispatch(pipeline *Pipeline, gpu_ptr<T> Root, u32 ThreadsX, u32 ThreadsY = 1, u32 ThreadsZ = 1)
{
    JA_ASSERT(Pipeline->GroupSize[0] != 0 && "not a compute pipeline");
    if(ThreadsX == 0 || ThreadsY == 0 || ThreadsZ == 0)
    {
        return;
    }
    u32 GroupsX = (ThreadsX + Pipeline->GroupSize[0] - 1) / Pipeline->GroupSize[0];
    u32 GroupsY = (ThreadsY + Pipeline->GroupSize[1] - 1) / Pipeline->GroupSize[1];
    u32 GroupsZ = (ThreadsZ + Pipeline->GroupSize[2] - 1) / Pipeline->GroupSize[2];
    DispatchGroupsUntyped(Pipeline, Root.Offset, GroupsX, GroupsY, GroupsZ);
}

constexpr size_t GPU_ALIGNMENT = 16;

template <typename T>
[[nodiscard]] gpu_ptr<T>
GpuPtr(gpu_heap *Heap, T *CpuPtr)
{
    u8 *Byte = static_cast<u8 *>(static_cast<void *>(CpuPtr));
    JA_ASSERT(Byte >= Heap->Arena.Base && Byte < Heap->Arena.Base + Heap->Arena.Size);

    u32 Offset = static_cast<u32>(Byte - Heap->Arena.Base);
    JA_ASSERT(Offset % 4 == 0);
    return {.Offset = Offset};
}

// Allocates Count T's from Arena, which must live inside Heap: &Heap->Arena for
// data written once, Frame->Arena for data that changes every frame.
// Returns {} when the arena is full.
template <typename T>
[[nodiscard]] gpu_cpu_range<T>
PushGpu(gpu_heap *Heap, mem_arena *Arena, u32 Count = 1)
{
    umm HeapStart  = reinterpret_cast<umm>(Heap->Arena.Base);
    umm HeapEnd    = HeapStart + Heap->Arena.Size;
    umm ArenaStart = reinterpret_cast<umm>(Arena->Base);
    umm ArenaEnd   = ArenaStart + Arena->Size;
    JA_ASSERT(ArenaStart >= HeapStart && ArenaEnd <= HeapEnd);

    constexpr size_t Alignment = alignof(T) > GPU_ALIGNMENT ? alignof(T) : GPU_ALIGNMENT;
    T               *Cpu       = static_cast<T *>(PushSize(Arena, sizeof(T) * Count, Alignment));
    if(!Cpu)
    {
        return {};
    }
    return {.Cpu = Cpu, .Gpu = GpuPtr(Heap, Cpu), .Count = Count};
}
