#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;

uniform float time;
uniform float pulsePhase;

uniform vec3 beamColor;
uniform float beamIntensity;
uniform float beamAlpha;
uniform float beamLength;

void main()
{
    float distanceFromBase = abs(FragPos.y);
    float falloff = 1.0 - smoothstep(0.0, beamLength, distanceFromBase);

    float intensity = pulsePhase * falloff * beamIntensity;

    vec3 result = beamColor * intensity;

    FragColor = vec4(result, beamAlpha);
    BrightColor = vec4(result, 1.0);
}