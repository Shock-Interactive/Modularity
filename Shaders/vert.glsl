#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform float uTime;
uniform float uSpeed;

void main()
{
    float angle = uTime * uSpeed;
    float c = cos(angle);
    float s = sin(angle);

    mat2 rot = mat2(c, -s,
                    s,  c);

    // Triangle vertices are in [-0.5, 0.5] range → centre is 0.
    vec2 rotated = rot * aPos.xy;

    gl_Position = vec4(rotated, aPos.z, 1.0);

    vertexColor = vec3(aPos.x + 0.5, aPos.y + 0.5, abs(aPos.x - aPos.y));
}