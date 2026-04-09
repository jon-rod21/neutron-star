#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

float hash(vec3 p) {
    p = fract(p * vec3(234.34, 435.345, 457.09));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

void main()
{    
    vec3 dir = normalize(TexCoords);
    
    // FIX 1: Grid Snapping (Stops the shimmering/recalculating effect)
    // This locks the math to invisible rigid chunks. 
    // Higher = tiny pinpoint stars. Lower = bigger, blocky stars.
    float resolution = 400.0; 
    vec3 gridId = floor(dir * resolution);
    
    // Generate the random value based on the GRID cell, not the fluid pixel
    float starValue = hash(gridId); 
    
    // FIX 2: Density Control
    // Changed from 0.99 to 0.998. Now, only the top 0.2% of space generates a star.
    // If it's still too dense, push it to 0.999!
    if(starValue > 0.998) { 
        
        // Recalculate brightness curve to match the new threshold
        float brightness = (starValue - 0.998) * 500.0; 
        vec3 starColor = vec3(brightness);
        
        FragColor = vec4(starColor, 1.0);
        
        // Push the brightest stars to the bloom buffer
        if(brightness > 0.8)
            BrightColor = vec4(starColor, 1.0);
        else
            BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}