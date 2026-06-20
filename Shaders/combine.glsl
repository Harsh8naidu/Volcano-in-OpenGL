#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D sceneTexture; // Original scene
uniform sampler2D bloomTexture; // Blurred bright areas

void main()
{
    vec3 hdrSceneColor = texture(sceneTexture, vTexCoord).rgb; // Original scene color
    vec3 bloomColor = texture(bloomTexture, vTexCoord).rgb; // Bloom color from blurred bright areas
    
    vec3 finalColor = hdrSceneColor + bloomColor; // Combine the original scene with the bloom effect

    // tone mapping (Reinhard)
    finalColor = finalColor / (finalColor + vec3(1.0)); // Simple Reinhard tone mapping

    FragColor = vec4(finalColor, 1.0); // Output the final color
}