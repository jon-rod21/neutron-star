#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in float heightFactor;

uniform vec3 fieldColor;
uniform float fieldAlpha;

void main()
{
    // Fade toward the poles and tips of the lines
    float fade = clamp(1.0 - heightFactor, 0.15, 1.0);
    vec3 col = fieldColor * fade;

    FragColor = vec4(col, fieldAlpha * fade);

    // Let bright areas bloom
    float brightness = dot(col, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = brightness > 0.4 ? vec4(col * 0.6, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
