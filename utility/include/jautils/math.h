#pragma once

#include "base.h"

#include <math.h>

struct alignas(8) vec2
{
    f32 X, Y;
};

inline vec2
operator+(vec2 A, vec2 B)
{
    return {A.X + B.X, A.Y + B.Y};
}

inline vec2
operator+=(vec2 &A, vec2 B)
{
    A = A + B;
    return A;
}

inline vec2
operator*(vec2 A, f32 S)
{
    return {A.X * S, A.Y * S};
}

inline f32
Length(vec2 A)
{
    return sqrtf(A.X * A.X + A.Y * A.Y);
}

struct alignas(16) vec4
{
    f32 X, Y, Z, W;
};

struct alignas(16) mat4
{
    vec4 Col[4];
};
static_assert(sizeof(mat4) == 64);

inline vec4
operator+(vec4 A, vec4 B)
{
    return {A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W};
}

inline vec4
operator*(vec4 A, f32 S)
{
    return {A.X * S, A.Y * S, A.Z * S, A.W * S};
}

inline vec4
operator*(f32 S, vec4 A)
{
    return A * S;
}

inline mat4
Identity()
{
    return {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    }};
}

inline vec4
operator*(mat4 M, vec4 V)
{
    return M.Col[0] * V.X + M.Col[1] * V.Y + M.Col[2] * V.Z + M.Col[3] * V.W;
}

inline mat4
operator*(mat4 A, mat4 B)
{
    return {{A * B.Col[0], A * B.Col[1], A * B.Col[2], A * B.Col[3]}};
}

inline mat4
RotationY(f32 Angle)
{
    f32 C = cosf(Angle);
    f32 S = sinf(Angle);
    return {{
        {C, 0, -S, 0},
        {0, 1, 0, 0},
        {S, 0, C, 0},
        {0, 0, 0, 1},
    }};
}

struct vec3
{
    f32 X, Y, Z;
};

inline vec3
operator-(vec3 A, vec3 B)
{
    return {A.X - B.X, A.Y - B.Y, A.Z - B.Z};
}

inline f32
Dot(vec3 A, vec3 B)
{
    return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

inline vec3
Cross(vec3 A, vec3 B)
{
    return {A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
}

inline vec3
Normalize(vec3 A)
{
    f32 InvLength = 1.0f / sqrtf(Dot(A, A));
    return {A.X * InvLength, A.Y * InvLength, A.Z * InvLength};
}

inline mat4
LookAt(vec3 Eye, vec3 Target, vec3 Up)
{
    vec3 F = Normalize(Target - Eye);
    vec3 S = Normalize(Cross(F, Up));
    vec3 U = Cross(S, F);
    return {{
        {S.X, U.X, -F.X, 0},
        {S.Y, U.Y, -F.Y, 0},
        {S.Z, U.Z, -F.Z, 0},
        {-Dot(S, Eye), -Dot(U, Eye), Dot(F, Eye), 1},
    }};
}

inline mat4
Perspective(f32 FovY, f32 Aspect, f32 Near, f32 Far)
{
    f32 F = 1.0f / tanf(FovY * 0.5f);
    return {{
        {F / Aspect, 0, 0, 0},
        {0, F, 0, 0},
        {0, 0, (Far + Near) / (Near - Far), -1},
        {0, 0, (2.0f * Far * Near) / (Near - Far), 0},
    }};
}
