#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;
uniform sampler2D texture2;

// Lighting uniforms
uniform vec3 lightPos;      // Position of the light in world space
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 objectColor = vec3(1.0, 1.0, 1.0);  // tint (optional)

uniform float ambientStrength = 0.1;
uniform float mixAmount = 0.2;  // how much of texture2 to blend

void main()
{
    // --- Texture sampling & mixing ---
    vec4 tex1 = texture(texture1, TexCoord);
    vec4 tex2 = texture(texture2, TexCoord);
    vec4 texColor = mix(tex1, tex2, mixAmount);

    // --- Ambient ---
    vec3 ambient = ambientStrength * lightColor;

    // --- Diffuse ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // --- Combine lighting ---
    vec3 lighting = ambient + diffuse;
    vec3 result = lighting * objectColor * texColor.rgb;

    FragColor = vec4(result, texColor.a);
}