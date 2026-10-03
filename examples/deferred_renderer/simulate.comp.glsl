#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "deferred_shared.h"

layout(local_size_x = 64) in;

void main()
{
    sim_args Args = LoadSimArgs(Root);
    uint I = gl_GlobalInvocationID.x;
    if(I >= Args.Count)
    {
        return;
    }

    uint GridWidth = uint(ceil(sqrt(float(Args.Count))));
    const float Spacing = 3.0;

    uint X = I % GridWidth;
    uint Z = I / GridWidth;

    uint Columns = min(Args.Count, GridWidth);
    uint Rows = (Args.Count + GridWidth - 1) / GridWidth;
    float HalfX = float(Columns - 1) * 0.5;
    float HalfZ = float(Rows - 1) * 0.5;
    vec3 Position = vec3((float(X) - HalfX) * Spacing, 0.0, (float(Z) - HalfZ) * Spacing);

    float Angle = Args.Time + float(I) * 0.1;

    mat4 Model = mat4(1.0);
    float C = cos(Angle);
    float S = sin(Angle);
    Model[0] = vec4(C, 0.0, -S, 0.0);
    Model[1] = vec4(0.0, 1.0, 0.0, 0.0);
    Model[2] = vec4(S, 0.0, C, 0.0);
    Model[3] = vec4(Position, 1.0);
    StoreMat4(Args.ObjectsPtr + I * MAT4_SIZE, Model);
}
