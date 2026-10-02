#ifndef CUBE_SHARED_H
#define CUBE_SHARED_H

#include "jagfx/jagfx_shared.h"

#define VERTEX_SIZE     32
#define VERTEX_POSITION 0
#define VERTEX_UV       16

#define DRAW_ARGS_SIZE         144
#define DRAW_ARGS_VIEWPROJ     0
#define DRAW_ARGS_MODEL        64
#define DRAW_ARGS_VERTICES_PTR 128
#define DRAW_ARGS_TEXTURE      136

#ifdef __cplusplus
#include <stddef.h>
#include <jautils/math.h>
#endif

struct vertex
{
    vec4 Position;
    vec2 UV;
};

struct draw_args
{
    mat4            ViewProj;
    mat4            Model;
    GPU_PTR(vertex) VerticesPtr;
    GPU_TEXTURE     Texture;
};

#ifdef __cplusplus
static_assert(sizeof(vertex) == VERTEX_SIZE);
static_assert(offsetof(vertex, Position) == VERTEX_POSITION);
static_assert(offsetof(vertex, UV) == VERTEX_UV);

static_assert(sizeof(draw_args) == DRAW_ARGS_SIZE);
static_assert(offsetof(draw_args, ViewProj) == DRAW_ARGS_VIEWPROJ);
static_assert(offsetof(draw_args, Model) == DRAW_ARGS_MODEL);
static_assert(offsetof(draw_args, VerticesPtr) == DRAW_ARGS_VERTICES_PTR);
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
    A.Model       = LoadMat4(Ptr + DRAW_ARGS_MODEL);
    A.VerticesPtr = LoadU32(Ptr + DRAW_ARGS_VERTICES_PTR);
    A.Texture     = LoadUVec2(Ptr + DRAW_ARGS_TEXTURE);
    return A;
}
#endif

#endif
