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
uniform float layerAlpha;
uniform vec3 viewPos;

float hash(vec3 p)
{
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash(i + vec3(0, 0, 0));
    float n100 = hash(i + vec3(1, 0, 0));
    float n010 = hash(i + vec3(0, 1, 0));
    float n110 = hash(i + vec3(1, 1, 0));
    float n001 = hash(i + vec3(0, 0, 1));
    float n101 = hash(i + vec3(1, 0, 1));
    float n011 = hash(i + vec3(0, 1, 1));
    float n111 = hash(i + vec3(1, 1, 1));

    float x00 = mix(n000, n100, f.x);
    float x10 = mix(n010, n110, f.x);
    float x01 = mix(n001, n101, f.x);
    float x11 = mix(n011, n111, f.x);

    float y0 = mix(x00, x10, f.y);
    float y1 = mix(x01, x11, f.y);

    return mix(y0, y1, f.z);
}

void main()
{
    float y = clamp(abs(LocalPos.y) / 5.0, 0.0, 1.0);

    // Strong near the neutron star, fades outward.
    float lengthFade = exp(-y * 0.65);

    // Prevent a harsh cut at the far end.
    lengthFade *= 1.0 - smoothstep(0.92, 1.0, y);

    // Animated plasma bands moving outward.
    float bands = sin(y * 34.0 - time * 8.0);
    bands = 0.65 + 0.35 * bands;

    // Flickering turbulent detail.
    float turbulence = noise(vec3(LocalPos.xz * 12.0, y * 6.0 - time * 1.5));
    turbulence = mix(0.75, 1.25, turbulence);

    // View-dependent glow so the beam feels volumetric.
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 norm = normalize(Normal);
    float rim = 1.0 - abs(dot(viewDir, norm));
    rim = pow(rim, 1.8);

    float pulse = mix(0.65, 1.25, pulsePhase);

    float intensity =
        beamIntensity *
        pulse *
        lengthFade *
        bands *
        turbulence *
        mix(0.75, 1.35, rim);

    vec3 coreColor = mix(beamColor, vec3(0.9, 0.97, 1.0), 0.35);
    vec3 result = coreColor * intensity;

    float alpha = beamAlpha * layerAlpha * lengthFade * mix(0.6, 1.0, rim);
    alpha = clamp(alpha, 0.0, 1.0);

    FragColor = vec4(result, alpha);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = brightness > 0.3 ? vec4(result, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}