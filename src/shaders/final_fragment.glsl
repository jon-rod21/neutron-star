#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

// Gravitational lensing
uniform vec2 starScreenPos;
uniform float starApparentRadius;
uniform float lensStrength;
uniform float lensCompactness;
uniform float aspectRatio;

void main()
{
    vec2 uv = TexCoords;

    vec2 offset = uv - starScreenPos;
    offset.x *= aspectRatio;

    float r = length(offset);
    float safeR = max(r, 0.00001);

    vec2 radialDir = offset / safeR;

    float starR = max(starApparentRadius, 0.0001);
    float compactness = clamp(lensCompactness, 0.2, 4.0);

    // Lens influence grows with compactness, but stays local around the star.
    float influenceRadius = starR * mix(3.0, 7.0, clamp(compactness / 4.0, 0.0, 1.0));
    influenceRadius = clamp(influenceRadius, starR * 2.5, 0.45);

    // Do not warp the visible star surface itself.
    float outsideStar = smoothstep(starR * 1.05, starR * 1.35, r);

    // Fade lensing out smoothly so the whole screen does not bend weirdly.
    float outerFade = 1.0 - smoothstep(influenceRadius * 0.65, influenceRadius, r);

    float lensMask = outsideStar * outerFade;

    // Normalized distance from the star limb.
    float rNorm = safeR / starR;

    // Stable screen-space approximation:
    // strongest near the limb, falls off smoothly with distance.
    float deflection = lensStrength * compactness * starR * (1.0 / max(rNorm, 1.0));

    // Clamp prevents extreme ugly stretching.
    deflection = min(deflection, starR * 0.35);

    deflection *= lensMask;

    // Bend background light toward the star.
    vec2 sampleOffset = -radialDir * deflection;
    sampleOffset.x /= aspectRatio;

    vec2 warpedUV = uv + sampleOffset;
    warpedUV = clamp(warpedUV, vec2(0.001), vec2(0.999));

    vec3 hdrColor = texture(scene, warpedUV).rgb;
    vec3 bloomColor = texture(bloomBlur, warpedUV).rgb;

    vec3 color = hdrColor + bloomColor;

    // Add a subtle Einstein/photon-ring style brightening near the limb.
    float ring = 1.0 - smoothstep(0.0, starR * 0.18, abs(r - starR * 1.18));
    ring *= outerFade;
    ring *= clamp(compactness / 3.0, 0.0, 1.0);
    ring *= lensStrength;

    color += vec3(0.35, 0.55, 1.0) * ring * 0.6;

    vec3 result = vec3(1.0) - exp(-color * exposure);
    FragColor = vec4(result, 1.0);
}