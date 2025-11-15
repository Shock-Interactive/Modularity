#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    fragPos = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Trick to ensure skybox depth is always 1.0 (furthest)
}
