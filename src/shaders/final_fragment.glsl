#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

// Gravitational lensing
uniform vec2 starScreenPos;       // star's projected center in [0,1] UV
uniform float starApparentRadius; // apparent radius in vertical UV units
uniform float lensStrength;       // overall bend factor
uniform float aspectRatio;        // width / height

void main()
{
    // Aspect-correct offset from star center so the lens stays circular
    vec2 offset = TexCoords - starScreenPos;
    offset.x *= aspectRatio;
    float r = length(offset);
    float safeR = max(r, 1e-5);
    vec2 outward = offset / safeR;

    // Einstein-radius-scaled 1/b deflection (Schwarzschild asymptotic form)
    float einsteinSq = starApparentRadius * starApparentRadius * lensStrength;
    float deflection = einsteinSq / safeR;

    // Don't warp inside the silhouette — surface stays intact, ramp in past the limb
    float silhouetteMask = smoothstep(starApparentRadius * 0.95, starApparentRadius * 1.15, r);
    deflection *= silhouetteMask;

    // Light bends toward the mass: sample the source closer to the center
    vec2 sampleOffset = -outward * deflection;
    sampleOffset.x /= aspectRatio;

    vec2 warpedUV = TexCoords + sampleOffset;

    vec3 hdrColor = texture(scene, warpedUV).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    hdrColor += bloomColor; // additive blending

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(result, 1.0);
}
