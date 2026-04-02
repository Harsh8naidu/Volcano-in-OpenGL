#version 330 core

in Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
    vec3 fragPos;
} IN;

out vec4 FragColor;        // Final color output

uniform sampler2D diffuseTexture; // Diffuse texture
uniform sampler2D roughnessTexture; // Roughness texture
uniform sampler2D metallicTexture; // Metallic texture

uniform float shininess;          // Shininess factor
uniform vec3 ambient;             // Ambient color
uniform vec3 specular;            // Specular color
uniform vec3 emission;            // Emission color
uniform float ior;                // Index of refraction
uniform float dissolveFactor;     // Dissolve factor for transparency
uniform int illuminationModel;    // Illumination model selector
uniform vec3 lightDir;            // Light direction (passed as a uniform)
uniform vec3 cameraPos;           // Camera position (passed as a uniform)

void main() {
    // Normalize the input normal
    vec3 norm = normalize(IN.normal);
    // Ambient component
    vec3 ambientComponent = ambient * 0.01;

    // Diffuse component
    vec3 diffuseColor = texture(diffuseTexture, IN.texCoord).rgb;

    // Roughness component
    float roughness = texture(roughnessTexture, IN.texCoord).r;

    // Metallic component
    float metallic = texture(metallicTexture, IN.texCoord).r;

    // Diffuse component based on the light direction and normal
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    diffuseColor *= diff * (1.0 - metallic);  // Diffuse contribution is reduced by metallic factor

    // Specular component
    vec3 viewDir = normalize(cameraPos - IN.fragPos);  // Assume the camera is at the origin
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float shininessValue = max(shininess * (1.0 - roughness), 0.1); // Adjust shininess based on roughness
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininessValue);
    vec3 specularComponent = spec * specular * (1.0 - metallic);

    // Combine the components based on the illumination model
    vec3 finalColor = ambientComponent + diffuseColor + specularComponent + emission;

    // Apply dissolve effect (transparency based on the dissolve factor)
    float alpha = dissolveFactor;

    FragColor = vec4(finalColor, alpha);  // Final color with transparency
}