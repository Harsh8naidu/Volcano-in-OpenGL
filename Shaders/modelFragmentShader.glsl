#version 330 core

#define MAX_LIGHTS 16

struct Light {
    vec3 position;
    vec4 color;
    float radius;
};

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
uniform vec3 cameraPos;           // Camera position (passed as a uniform)

// Multi-light uniforms 
uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

void main() {
    // Normalize the input normal
    vec3 norm = normalize(IN.normal);
    vec3 viewDir = normalize(cameraPos - IN.fragPos);  // Assume the camera is at the origin
    vec3 diffuseColor = texture(diffuseTexture, IN.texCoord).rgb;
    float roughness = texture(roughnessTexture, IN.texCoord).r;
    float metallic = texture(metallicTexture, IN.texCoord).r;

    float shininessValue = max(shininess * (1.0 - roughness), 0.1);
    
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount; i++) {
        vec3 incident = normalize(lights[i].position - IN.fragPos);
        vec3 halfDir = normalize(incident + viewDir);
        float distance = length(lights[i].position - IN.fragPos);
        float attenuation = 1.0 - clamp(distance / lights[i].radius, 0.0, 1.0);

        // Diffuse
        float diff = max(dot(norm, incident), 0.0);
        vec3 diffuseContrib = diffuseColor * lights[i].color.rgb * diff * (1.0 - metallic);
        
        // Specular
        float spec = pow(max(dot(norm, halfDir), 0.0), shininessValue);
        vec3 specularContrib = specular * lights[i].color.rgb * spec * (1.0 - metallic) * 0.33;

        result += (diffuseContrib + specularContrib) * attenuation;
    }

    // Ambient + emission (applied once, not per light)
        result += ambient * diffuseColor * 0.5;
        result += emission;

        FragColor = vec4(result, dissolveFactor);
}