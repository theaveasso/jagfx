#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "cube_shared.h"

out vec2 UV;
out vec3 WorldPos;

void main()
{
    draw_args Args = LoadDrawArgs(Root);
    vertex Vertex = LoadVertex(Args.VerticesPtr + gl_VertexID * VERTEX_SIZE);

    gl_Position = Args.ViewProj * Args.Model * Vertex.Position;
    UV = Vertex.UV;
    WorldPos = (Args.Model * Vertex.Position).xyz;
}
