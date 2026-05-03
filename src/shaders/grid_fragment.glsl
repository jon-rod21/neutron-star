#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in float distFromCenter;
uniform float gravSoftening;

void main()
{
    //Fade grid lines out at distance, glow near center
    float fade = clamp(1.0 - distFromCenter / 50.0, 0.2, 1.0);
    float glow = clamp(gravSoftening / (distFromCenter + gravSoftening), 0.0, 1.0);
    
    vec3 color = mix(vec3(0.1, 0.3, 0.6), vec3(0.4, 0.8, 1.0), glow) * fade;

    FragColor = vec4(color, fade * 0.8);
    // FragColor = vec4(0.4, 0.8, 1.0, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0); // don't bloom the grid
}
