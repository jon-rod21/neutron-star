#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // Apply projection and view, but strip the translation from the view matrix in C++
    vec4 pos = projection * view * vec4(aPos, 1.0);
    
    // pos.xyww forces the depth (z) to always equal the w component (which becomes 1.0 after perspective division). 
    // This pushes the skybox to the absolute maximum depth of the screen.
    gl_Position = pos.xyww; 
}