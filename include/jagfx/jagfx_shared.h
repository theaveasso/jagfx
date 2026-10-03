#ifndef JAGFX_SHARED_H
#define JAGFX_SHARED_H

#ifdef __cplusplus
#include <stddef.h>
#include <jautils/base.h>

template <typename T>
struct gpu_ptr
{
    u32 Offset;
};

#define GPU_PTR(T)  gpu_ptr<T>
#define GPU_TEXTURE u64
#else
#extension GL_ARB_bindless_texture : require

#define GPU_PTR(T)  uint
#define GPU_TEXTURE uvec2
#define u32         uint
#define f32         float
#define s32         int

#define MAT4_SIZE 64
#endif

#define DRAW_INDEXED_COMMAND_SIZE           20
#define DRAW_INDEXED_COMMAND_INDEX_COUNT    0
#define DRAW_INDEXED_COMMAND_INSTANCE_COUNT 4
#define DRAW_INDEXED_COMMAND_FIRST_INDEX    8
#define DRAW_INDEXED_COMMAND_BASE_VERTEX    12
#define DRAW_INDEXED_COMMAND_BASE_INSTANCE  16

struct draw_indexed_command
{
    u32 IndexCount;
    u32 InstanceCount;
    u32 FirstIndex;
    s32 BaseVertex;
    u32 BaseInstance;
};

#ifdef __cplusplus
static_assert(sizeof(gpu_ptr<u32>) == 4);

static_assert(sizeof(draw_indexed_command) == DRAW_INDEXED_COMMAND_SIZE);
static_assert(offsetof(draw_indexed_command, IndexCount) == DRAW_INDEXED_COMMAND_INDEX_COUNT);
static_assert(offsetof(draw_indexed_command, InstanceCount) == DRAW_INDEXED_COMMAND_INSTANCE_COUNT);
static_assert(offsetof(draw_indexed_command, FirstIndex) == DRAW_INDEXED_COMMAND_FIRST_INDEX);
static_assert(offsetof(draw_indexed_command, BaseVertex) == DRAW_INDEXED_COMMAND_BASE_VERTEX);
static_assert(offsetof(draw_indexed_command, BaseInstance) == DRAW_INDEXED_COMMAND_BASE_INSTANCE);
#else
// clang-format off
layout(std430, binding = 0) buffer JagfxHeapU32  { uint HeapU32[]; };
layout(std430, binding = 0) buffer JagfxHeapVec4 { vec4 HeapVec4[]; };

layout(location = 0) uniform uint Root;

uint  LoadU32(uint Ptr)   { return HeapU32[Ptr / 4]; }
float LoadF32(uint Ptr)   { return uintBitsToFloat(HeapU32[Ptr / 4]); }
uvec2 LoadUVec2(uint Ptr) { return uvec2(LoadU32(Ptr + 0), LoadU32(Ptr + 4)); }
vec2  LoadVec2(uint Ptr)  { return vec2(LoadF32(Ptr + 0), LoadF32(Ptr + 4)); }
vec4  LoadVec4(uint Ptr)  { return HeapVec4[Ptr / 16]; }
mat4  LoadMat4(uint Ptr)  { return mat4(LoadVec4(Ptr + 0), LoadVec4(Ptr + 16), LoadVec4(Ptr + 32), LoadVec4(Ptr + 48)); }

void  StoreU32(uint Ptr, uint Value)  { HeapU32[Ptr / 4] = Value; }
void  StoreF32(uint Ptr, float Value) { HeapU32[Ptr / 4] = floatBitsToUint(Value); }
void  StoreVec4(uint Ptr, vec4 Value) { StoreF32(Ptr + 0, Value.x); StoreF32(Ptr + 4, Value.y); StoreF32(Ptr + 8, Value.z); StoreF32(Ptr + 12, Value.w); }
void  StoreMat4(uint Ptr, mat4 Value) { StoreVec4(Ptr + 0, Value[0]); StoreVec4(Ptr + 16, Value[1]); StoreVec4(Ptr + 32, Value[2]); StoreVec4(Ptr + 48, Value[3]); }

uint AtomicAddU32(uint Ptr, uint Value) { return atomicAdd(HeapU32[Ptr / 4], Value); }
// clang-format on
#endif
#endif
