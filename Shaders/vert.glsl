#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vertexColor = vec3(aPos.x + 0.5, aPos.y + 0.5, abs(aPos.x - aPos.y));
}