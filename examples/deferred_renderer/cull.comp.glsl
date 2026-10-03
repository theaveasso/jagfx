#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "deferred_shared.h"

layout(local_size_x = 64) in;

void main()
{
    cull_args Args = LoadCullArgs(Root);
    uint I = gl_GlobalInvocationID.x;
    if(I >= Args.Count)
    {
        return;
    }

    mat4 Model = LoadMat4(Args.ObjectsPtr + I * MAT4_SIZE);
    vec3 Position = vec3(Model[3].xyz);
    if (Position.x > 0)
    {
        return;
    }

    uint Slot = AtomicAddU32(Args.CommandPtr + DRAW_INDEXED_COMMAND_INSTANCE_COUNT, 1);
    StoreU32(Args.VisibleIdsPtr + Slot * 4, I);
}
