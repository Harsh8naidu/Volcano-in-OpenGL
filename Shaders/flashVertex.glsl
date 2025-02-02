#version 330 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec2 texCoord; // Texture coordinates

out vec2 fragTexCoord; // Pass texture coordinates to the fragment shader

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    fragTexCoord = texCoord; // Pass the texture coordinate
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0); // Calculate clip space position
}