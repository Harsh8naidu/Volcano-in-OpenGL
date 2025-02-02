#version 330 core

in vec2 fragTexCoord; // Interpolated texture coordinates from vertex shader

out vec4 fragColor; // Final output color

uniform sampler2D flashTex; // Full-screen texture
uniform float flashIntensity = 1.0; // Flash intensity (can be animated if needed)

void main() {
    vec4 texColor = texture(flashTex, fragTexCoord); // Sample texture color
    fragColor = texColor * flashIntensity; // Apply flash intensity
}