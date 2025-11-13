#version 330 core
out vec4 FragColor;
in vec3 vertexColor;

uniform float time;

void main()
{
    vec3 color = 0.5 + 0.5 * cos(time + vertexColor * 6.2831);
    FragColor = vec4(color, 1.0);
}