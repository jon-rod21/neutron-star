#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model; // carries star rotation

out float heightFactor; // fade based on distance from equator

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    heightFactor = abs(aPos.y) / 5.0; // rough fade along axis
    gl_Position = projection * view * worldPos;
}
