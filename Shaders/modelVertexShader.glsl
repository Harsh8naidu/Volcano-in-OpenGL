#version 330 core

layout(location = 0) in vec3 position;  // Vertex position
layout(location = 1) in vec2 texCoord;  // Vertex texture coordinates
layout(location = 2) in vec3 normal;    // Vertex normal

uniform mat4 modelMatrix;   // Model matrix
uniform mat4 viewMatrix;    // View matrix
uniform mat4 projMatrix;    // Projection matrix
uniform vec4 nodeColour;    // Node color (can be used for vertex coloring if needed)

out Vertex {
    vec2 texCoord;          // Texture coordinates to pass to the fragment shader
    vec4 colour;            // Color to pass to the fragment shader (if needed)
    vec3 normal;            // Normal to pass to the fragment shader
    vec3 fragPos;           // Fragment position (to pass to the fragment shader)
} OUT;

void main(void) {
    // Transform the vertex position to clip space using the model, view, and projection matrices
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

    // Pass data to the fragment shader
    OUT.texCoord = texCoord;
    OUT.colour = nodeColour;  // If needed for vertex color, can be used in the fragment shader
    //OUT.normal = normalize(mat3(transpose(inverse(modelMatrix))) * normal);  // Transform normal to world space
    OUT.normal = normalize(mat3(transpose(inverse(modelMatrix))) * normal);  // Transform normal to world space
    OUT.fragPos = vec3(modelMatrix * vec4(position, 1.0));  // Position in world space
}