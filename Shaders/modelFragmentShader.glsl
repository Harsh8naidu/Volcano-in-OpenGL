#version 330 core

in vec2 fragTexCoord;  // Texture coordinates from the vertex shader

out vec4 fragColor;    // Final color output

uniform sampler2D modelTexture;  // Texture sampler

void main() {
    // Sample the texture at the given texture coordinates
    fragColor = texture(modelTexture, fragTexCoord);
}