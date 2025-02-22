#version 330 core

in vec2 texCoord;          // Texture coordinates passed from vertex shader
in vec3 normal;            // Normal passed from vertex shader
in vec3 fragPos;           // World space fragment position passed from vertex shader

out vec4 FragColor;        // Final color output

uniform sampler2D diffuseTexture; // Diffuse texture
uniform float shininess;          // Shininess factor
uniform vec3 ambient;             // Ambient color
uniform vec3 specular;            // Specular color
uniform vec3 emission;            // Emission color
uniform float ior;                // Index of refraction
uniform float dissolveFactor;     // Dissolve factor for transparency
uniform int illuminationModel;    // Illumination model selector
uniform vec3 lightDir;            // Light direction (passed as a uniform)

void main() {
    // Normalize the input normal
    vec3 norm = normalize(normal);

    // Ambient component
    vec3 ambientComponent = ambient;

    // Diffuse component
    vec3 diffuseColor = texture(diffuseTexture, texCoord).rgb;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseComponent = diff * diffuseColor;

    // Specular component
    vec3 viewDir = normalize(-fragPos);  // Assume the camera is at the origin
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularComponent = spec * specular;

    // Emission component
    vec3 emissionComponent = emission;

    // Combine the components based on the illumination model
    vec3 finalColor = ambientComponent + diffuseComponent + specularComponent + emissionComponent;

    // Apply dissolve effect (transparency based on the dissolve factor)
    float alpha = 1.0 - dissolveFactor;

    FragColor = vec4(finalColor, 1.0);  // Final color with transparency
}