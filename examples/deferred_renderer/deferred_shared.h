#ifndef DEFERRED_SHARED_H
#define DEFERRED_SHARED_H

#include "jagfx/jagfx_shared.h"

#define VERTEX_SIZE     32
#define VERTEX_POSITION 0
#define VERTEX_UV       16

#define SIM_ARGS_SIZE        12
#define SIM_ARGS_COUNT       0
#define SIM_ARGS_TIME        4
#define SIM_ARGS_OBJECTS_PTR 8

#define DRAW_ARGS_SIZE         80
#define DRAW_ARGS_VIEWPROJ     0
#define DRAW_ARGS_VERTICES_PTR 64
#define DRAW_ARGS_OBJECTS_PTR  68
#define DRAW_ARGS_TEXTURE      72

#ifdef __cplusplus
#include <stddef.h>
#include <jautils/math.h>
#else
#endif

struct vertex
{
    vec4 Position;
    vec2 UV;
};

struct sim_args
{
    u32           Count;
    f32           Time;
    GPU_PTR(mat4) ObjectsPtr;
};

struct draw_args
{
    mat4            ViewProj;
    GPU_PTR(vertex) VerticesPtr;
    GPU_PTR(mat4)   ObjectsPtr;
    GPU_TEXTURE     Texture;
};

#ifdef __cplusplus
static_assert(sizeof(vertex) == VERTEX_SIZE);
static_assert(offsetof(vertex, Position) == VERTEX_POSITION);
static_assert(offsetof(vertex, UV) == VERTEX_UV);

static_assert(sizeof(sim_args) == SIM_ARGS_SIZE);
static_assert(offsetof(sim_args, Count) == SIM_ARGS_COUNT);
static_assert(offsetof(sim_args, Time) == SIM_ARGS_TIME);
static_assert(offsetof(sim_args, ObjectsPtr) == SIM_ARGS_OBJECTS_PTR);

static_assert(sizeof(draw_args) == DRAW_ARGS_SIZE);
static_assert(offsetof(draw_args, ViewProj) == DRAW_ARGS_VIEWPROJ);
static_assert(offsetof(draw_args, VerticesPtr) == DRAW_ARGS_VERTICES_PTR);
static_assert(offsetof(draw_args, ObjectsPtr) == DRAW_ARGS_OBJECTS_PTR);
static_assert(offsetof(draw_args, Texture) == DRAW_ARGS_TEXTURE);
#else

vertex
LoadVertex(uint Ptr)
{
    vertex V;
    V.Position = LoadVec4(Ptr + VERTEX_POSITION);
    V.UV       = LoadVec2(Ptr + VERTEX_UV);
    return V;
}

draw_args
LoadDrawArgs(uint Ptr)
{
    draw_args A;
    A.ViewProj    = LoadMat4(Ptr + DRAW_ARGS_VIEWPROJ);
    A.VerticesPtr = LoadU32(Ptr + DRAW_ARGS_VERTICES_PTR);
    A.ObjectsPtr  = LoadU32(Ptr + DRAW_ARGS_OBJECTS_PTR);
    A.Texture     = LoadUVec2(Ptr + DRAW_ARGS_TEXTURE);
    return A;
}

sim_args
LoadSimArgs(uint Ptr)
{
    sim_args A;
    A.Count     = LoadU32(Ptr + SIM_ARGS_COUNT);
    A.Time      = LoadF32(Ptr + SIM_ARGS_TIME);
    A.ObjectsPtr = LoadU32(Ptr + SIM_ARGS_OBJECTS_PTR);
    return A;
}
#endif

#endif
