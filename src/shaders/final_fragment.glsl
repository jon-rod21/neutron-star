#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

// Gravitational lensing
uniform vec2 starScreenPos;       // star center in [0,1] UV
uniform float starApparentRadius; // apparent radius in vertical UV units
uniform float lensStrength;
uniform float aspectRatio;

void main()
{
    vec2 offset = TexCoords - starScreenPos;
    offset.x *= aspectRatio; // make distance metric circular
    float r = length(offset);
    float safeR = max(r, 1e-5);
    vec2 outward = offset / safeR;

    // Warp only outside the star's silhouette
    // Ramp up over a band that extends 2x the star radius beyond the limb
    float outerEdge = starApparentRadius * 3.0;
    float silhouetteMask = smoothstep(starApparentRadius, outerEdge, r);

    // 1/r deflection — light bends toward mass
    // lensStrength controls how many UV units the rays bend at the limb
    float deflection = (lensStrength * starApparentRadius) / safeR;
    deflection *= silhouetteMask;

    // Bend toward star center (negative = inward)
    vec2 sampleOffset = -outward * deflection;
    sampleOffset.x /= aspectRatio; // undo aspect correction for sampling

    vec2 warpedUV = TexCoords + sampleOffset;

    vec3 hdrColor = texture(scene, warpedUV).rgb;
    vec3 bloomColor = texture(bloomBlur, warpedUV).rgb;
    hdrColor += bloomColor;

    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    FragColor = vec4(result, 1.0);
}
