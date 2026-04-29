#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;

uniform float time;
uniform float pulsePhase;

void main()
{
    vec3 beamColor = vec3(0.6, 0.8, 1.0);

    float distanceFromBase = FragPos.y;
    float falloff = 1.0 - smoothstep(0.0, 5.0, abs(distanceFromBase));
    
    float intensity = pulsePhase * falloff * 5.0;

    vec3 result = beamColor * intensity;

    FragColor = vec4(result, intensity * 0.3);
    BrightColor = vec4(result, 1.0);
}

    

