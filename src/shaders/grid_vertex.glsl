#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 starPos;
uniform float gravStrength;  // controls warp depth
uniform float gravSoftening; // prevents singularity at center

out float distFromCenter; // pass to fragment for color fade

void main()
{
     vec3 pos = aPos;
    
     // Horizontal distance from star on XZ plane
     vec2 offset = pos.xz - starPos.xz;
     float dist = length(offset);
    
     // Gravity well displacement downward
     pos.y -= gravStrength / (dist + gravSoftening);
    
    
     distFromCenter = dist;

    gl_Position = projection * view * vec4(pos, 1.0);
}
