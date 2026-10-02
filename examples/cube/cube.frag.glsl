#version 460 core

#include "cube_shared.h"

in vec2 UV;
in vec3 WorldPos;

out vec4 Color;

void main()
{
    draw_args Args = LoadDrawArgs(Root);
    sampler2D Tex = sampler2D(Args.Texture);

    vec4 TexColor = texture(Tex, UV);

    vec3 Dx = dFdx(WorldPos);
    vec3 Dy = dFdy(WorldPos);
    vec3 N = normalize(cross(Dx, Dy));

    vec3 L = normalize(vec3(1.0, 0.0, 1.0));

    float Ambient = 0.2;
    float Diffuse = max(dot(N, L), 0.0);
    float Lighting = Ambient + Diffuse * (1.0 - Ambient);

    Color = vec4(TexColor.xyz * Lighting, TexColor.a);
}
