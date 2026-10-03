#ifndef JAGFX_SHARED_H
#define JAGFX_SHARED_H

#ifdef __cplusplus
#include <jautils/base.h>

template <typename T>
struct gpu_ptr
{
    u32 Offset;
};
static_assert(sizeof(gpu_ptr<u32>) == 4);
#define GPU_PTR(T)  gpu_ptr<T>
#define GPU_TEXTURE u64
#else

#extension GL_ARB_bindless_texture : require

#define GPU_PTR(T)  uint
#define GPU_TEXTURE uvec2
#define u32         uint
#define f32         float

#define MAT4_SIZE 64

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
// clang-format on
#endif
#endif
