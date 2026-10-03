#version 460 core

in vec2 UV;
in vec3 WorldPos;

out vec4 Color;

void main()
{
    vec3 Dx = dFdx(WorldPos);
    vec3 Dy = dFdy(WorldPos);
    vec3 N = normalize(cross(Dx, Dy));
    vec3 L = normalize(vec3(1.0, 0.0, 1.0));

    float Ambient = 0.2;
    float Diffuse = max(dot(N, L), 0.0);
    float Lighting = Ambient + Diffuse * (1.0 - Ambient);

    vec3 Base = vec3(UV, 0.5);
    Color = vec4(Base * Lighting, 1.0);
}
