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

#define CULL_ARGS_SIZE            16
#define CULL_ARGS_COUNT           0
#define CULL_ARGS_OBJECTS_PTR     4
#define CULL_ARGS_VISIBLE_IDS_PTR 8
#define CULL_ARGS_COMMAND_PTR     12

#define DRAW_ARGS_SIZE            80
#define DRAW_ARGS_VIEWPROJ        0
#define DRAW_ARGS_VERTICES_PTR    64
#define DRAW_ARGS_OBJECTS_PTR     68
#define DRAW_ARGS_VISIBLE_IDS_PTR 72

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

struct cull_args
{
    u32                           Count;
    GPU_PTR(mat4)                 ObjectsPtr;
    GPU_PTR(u32)                  VisibleIdsPtr;
    GPU_PTR(draw_indexed_command) CommandPtr;
};

struct draw_args
{
    mat4            ViewProj;
    GPU_PTR(vertex) VerticesPtr;
    GPU_PTR(mat4)   ObjectsPtr;
    GPU_PTR(u32)    VisibleIdsPtr;
};

#ifdef __cplusplus
static_assert(sizeof(vertex) == VERTEX_SIZE);
static_assert(offsetof(vertex, Position) == VERTEX_POSITION);
static_assert(offsetof(vertex, UV) == VERTEX_UV);

static_assert(sizeof(sim_args) == SIM_ARGS_SIZE);
static_assert(offsetof(sim_args, Count) == SIM_ARGS_COUNT);
static_assert(offsetof(sim_args, Time) == SIM_ARGS_TIME);
static_assert(offsetof(sim_args, ObjectsPtr) == SIM_ARGS_OBJECTS_PTR);

static_assert(sizeof(cull_args) == CULL_ARGS_SIZE);
static_assert(offsetof(cull_args, Count) == CULL_ARGS_COUNT);
static_assert(offsetof(cull_args, ObjectsPtr) == CULL_ARGS_OBJECTS_PTR);
static_assert(offsetof(cull_args, VisibleIdsPtr) == CULL_ARGS_VISIBLE_IDS_PTR);
static_assert(offsetof(cull_args, CommandPtr) == CULL_ARGS_COMMAND_PTR);

static_assert(sizeof(draw_args) == DRAW_ARGS_SIZE);
static_assert(offsetof(draw_args, ViewProj) == DRAW_ARGS_VIEWPROJ);
static_assert(offsetof(draw_args, VerticesPtr) == DRAW_ARGS_VERTICES_PTR);
static_assert(offsetof(draw_args, ObjectsPtr) == DRAW_ARGS_OBJECTS_PTR);
static_assert(offsetof(draw_args, VisibleIdsPtr) == DRAW_ARGS_VISIBLE_IDS_PTR);
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
    A.ViewProj      = LoadMat4(Ptr + DRAW_ARGS_VIEWPROJ);
    A.VerticesPtr   = LoadU32(Ptr + DRAW_ARGS_VERTICES_PTR);
    A.ObjectsPtr    = LoadU32(Ptr + DRAW_ARGS_OBJECTS_PTR);
    A.VisibleIdsPtr = LoadU32(Ptr + DRAW_ARGS_VISIBLE_IDS_PTR);
    return A;
}

cull_args
LoadCullArgs(uint Ptr)
{
    cull_args A;
    A.Count         = LoadU32(Ptr + CULL_ARGS_COUNT);
    A.ObjectsPtr    = LoadU32(Ptr + CULL_ARGS_OBJECTS_PTR);
    A.VisibleIdsPtr = LoadU32(Ptr + CULL_ARGS_VISIBLE_IDS_PTR);
    A.CommandPtr    = LoadU32(Ptr + CULL_ARGS_COMMAND_PTR);
    return A;
}

sim_args
LoadSimArgs(uint Ptr)
{
    sim_args A;
    A.Count      = LoadU32(Ptr + SIM_ARGS_COUNT);
    A.Time       = LoadF32(Ptr + SIM_ARGS_TIME);
    A.ObjectsPtr = LoadU32(Ptr + SIM_ARGS_OBJECTS_PTR);
    return A;
}
#endif

#endif
