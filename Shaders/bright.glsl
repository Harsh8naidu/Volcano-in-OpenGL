#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D sceneTexture;

void main()
{
    vec3 color = texture(sceneTexture, vTexCoord).rgb;

    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // Calculate brightness using luminance formula
    
    if (brightness > 1.0) // Threshold the brightness to isolate bright areas (Can be tuned as needed)
        FragColor = vec4(color, 1.0); // Keep bright areas
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Discard dark areas
}