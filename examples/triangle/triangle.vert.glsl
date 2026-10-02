#version 460 core
layout(std430, binding = 0) readonly buffer HeapBuffer { uint Heap[]; };
layout(location = 0) uniform uint Root;

vec2 LoadVec2(uint Index) {
    return vec2(uintBitsToFloat(Heap[Index]), uintBitsToFloat(Heap[Index + 1]));
}

void main()
{
    uint A = Root / 4;
    vec2 CameraPos = LoadVec2(A + 0);
    vec2 HalfSize = LoadVec2(A + 2);
    uint Vertices = Heap[A + 4] / 4;

    vec2 World = LoadVec2(Vertices + uint(gl_VertexID) * 2);
    vec2 Clip = (World - CameraPos) / HalfSize;
    gl_Position = vec4(Clip, 0.0, 1.0);
}
