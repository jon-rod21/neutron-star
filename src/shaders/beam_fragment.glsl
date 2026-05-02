#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 LocalPos;
in vec3 Normal;

uniform float time;
uniform float pulsePhase;

uniform vec3 beamColor;
uniform float beamIntensity;
uniform float beamAlpha;

void main()
{
    float distanceFromBase = abs(LocalPos.y);

    float falloff = 1.0 - smoothstep(0.0, 5.0, distanceFromBase);

    float intensity = pulsePhase * falloff * beamIntensity;

    vec3 result = beamColor * intensity;

    FragColor = vec4(result, beamAlpha);
    BrightColor = vec4(result, 1.0);
}