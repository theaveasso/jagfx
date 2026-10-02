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

// clang-format off
layout(std430, binding = 0) readonly buffer JagfxHeapU32    { uint HeapU32[]; };
layout(std430, binding = 0) readonly buffer JagfxHeapVec4   { vec4 HeapVec4[]; };

layout(location = 0) uniform uint Root;

uint  LoadU32(uint Ptr)   { return HeapU32[Ptr / 4]; }
float LoadF32(uint Ptr)   { return uintBitsToFloat(HeapU32[Ptr / 4]); }
uvec2 LoadUVec2(uint Ptr) { return uvec2(LoadU32(Ptr + 0), LoadU32(Ptr + 4)); }
vec2  LoadVec2(uint Ptr)  { return vec2(LoadF32(Ptr + 0), LoadF32(Ptr + 4)); }
vec4  LoadVec4(uint Ptr)  { return HeapVec4[Ptr / 16]; }
mat4  LoadMat4(uint Ptr)  { return mat4(LoadVec4(Ptr + 0), LoadVec4(Ptr + 16), LoadVec4(Ptr + 32), LoadVec4(Ptr + 48)); }
// clang-format on
#endif
#endif
