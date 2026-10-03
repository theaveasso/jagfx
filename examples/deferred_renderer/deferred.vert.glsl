#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "deferred_shared.h"

out vec2 UV;
out vec3 WorldPos;

void main()
{
    draw_args Args = LoadDrawArgs(Root);

    u32 Id = LoadU32(Args.VisibleIdsPtr + gl_InstanceID * 4);

    vertex Vertex = LoadVertex(Args.VerticesPtr + gl_VertexID * VERTEX_SIZE);
    UV = Vertex.UV;

    mat4 Model = LoadMat4(Args.ObjectsPtr + Id * MAT4_SIZE);

    gl_Position = Args.ViewProj * Model * Vertex.Position;
    WorldPos = (Model * Vertex.Position).xyz;
}
