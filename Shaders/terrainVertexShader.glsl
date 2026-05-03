#version 330 core

in vec3 position;  // Vertex position
in vec2 texCoord;  // Tiles UVs for diffuse textures
in vec3 normal;    // Vertex normal

uniform mat4 modelMatrix;   // Model matrix
uniform mat4 viewMatrix;    // View matrix
uniform mat4 projMatrix;    // Projection matrix
uniform mat3 normalMatrix;  // Normal matrix (for transforming normals correctly)
uniform sampler2D heightMap; // Height map for vertex displacement
uniform float heightScale;   // Scale factor for height displacement

out Vertex {
    vec2 texCoord;          // Texture coordinates to pass to the fragment shader
    vec3 normal;            // Normal to pass to the fragment shader
    vec3 fragPos;           // Fragment position (to pass to the fragment shader)
} OUT;

void main(void) {
    vec3 displacedPos = position;

    /* Note: Height displacement happens on the CPU side */

    // Calculate the final position of the vertex in clip space
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(displacedPos, 1.0);

    // Pass texture coordinates, normal, and fragment position to the fragment shader
    OUT.texCoord = texCoord;
    OUT.normal = normalize(normalMatrix * normal); // Transform the normal using the normal matrix
    OUT.fragPos = vec3(modelMatrix * vec4(displacedPos, 1.0)); // Calculate the world position of the fragment
}