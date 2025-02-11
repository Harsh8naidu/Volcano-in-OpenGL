#version 330 core

in vec2 texCoord;          // Texture coordinates passed from vertex shader
in vec3 normal;            // Normal passed from vertex shader
in vec3 fragPos;           // World space fragment position passed from vertex shader

out vec4 FragColor;        // Final color output

uniform sampler2D diffuseTexture; // Diffuse texture
uniform float shininess;          // Shininess factor
uniform vec3 ambient;        // Ambient color
uniform vec3 specular;       // Specular color
uniform vec3 emission;       // Emission color
uniform float ior;                // Index of refraction
uniform float dissolveFactor;     // Dissolve factor for transparency
uniform int illuminationModel;    // Illumination model selector

void main() {
    // Ambient component
    vec3 ambient = ambientColor;

    // Diffuse component
    vec3 diffuse = (texture(diffuseTexture, texCoord).rgb != vec3(0.0)) ? texture(diffuseTexture, texCoord).rgb : vec3(1.0);

    // Specular component
    vec3 viewDir = normalize(-fragPos);  // View direction (camera at the origin)
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));  // Example light direction
    vec3 reflectDir = reflect(-lightDir, normalize(normal));  // Reflection direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);  // Specular intensity
    vec3 specular = spec * specularColor;

    // Emission component
    vec3 emission = emissionColor;

    // Combine the components based on the illumination model
    vec3 finalColor = ambient + diffuse + specular + emission;

    // Apply dissolve effect (transparency based on the dissolve factor)
    float alpha = 1.0 - dissolveFactor;

    FragColor = vec4(finalColor, alpha);  // Final color with transparency
}
