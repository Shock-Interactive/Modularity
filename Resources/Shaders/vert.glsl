#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;   // NEW: normal attribute
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;      // Position in world space
out vec3 Normal;       // Normal in world space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // World-space position
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    // Transform normal to world space (non-uniform scale safe)
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Clip-space position
    gl_Position = projection * view * worldPos;

    TexCoord = aTexCoord;
}