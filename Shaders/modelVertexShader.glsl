#version 330 core

layout(location = 0) in vec3 vertexPosition;  // Vertex position
layout(location = 1) in vec2 vertexTexCoord;  // Texture coordinates
layout(location = 2) in vec3 vertexNormal;    // Vertex normal (if needed)

out vec2 fragTexCoord;  // Pass texture coordinates to the fragment shader

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    // Apply the transformation to the vertex position
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
    
    // Pass texture coordinates to the fragment shader
    fragTexCoord = vertexTexCoord;
}