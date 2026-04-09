#version 330 core

struct Light {
    vec3 position;
    vec4 color;
    float radius;
};

in Vertex {
    vec2 texCoord;          // Texture coordinates from the vertex shader
    vec3 normal;            // Normal vector from the vertex shader
    vec3 fragPos;           // Fragment position from the vertex shader
} IN;

out vec4 FragColor;        // Final color output

uniform sampler2D diffuseTex; // Diffuse texture
uniform vec3 cameraPos;           // Camera position (passed as a uniform)

// Multi-light uniforms
uniform Light lights[16];
uniform int lightCount;

void main() {
    vec3 norm = normalize(IN.normal);
    vec3 viewDir = normalize(cameraPos - IN.fragPos);  // Assume the camera is at the origin
    vec4 diffuseColor = texture(diffuseTex, IN.texCoord);
    vec3 result = diffuseColor.rgb * 0.5; // Ambient term (you can adjust this value)

    for (int i = 0; i < lightCount; i++) {
        vec3 incident = normalize(lights[i].position - IN.fragPos);
        vec3 halfDir = normalize(incident + viewDir);
        float distance = length(lights[i].position - IN.fragPos);
        //float attenuation = 1.0 - clamp(distance / lights[i].radius, 0.0, 1.0);
        float attenuation = 1.0;

        // Diffuse
        float diff = max(dot(norm, incident), 0.0);
        vec3 diffuseContrib = diffuseColor.rgb * lights[i].color.rgb * diff * attenuation;
        result += diffuseContrib;

        // Specular
        // float shininessValue = 1.0; // You can adjust this value or pass
        // float spec = pow(max(dot(norm, halfDir), 0.0), shininessValue);
        // vec3 specularContrib = lights[i].color.rgb * spec * attenuation;
        // result += specularContrib;
    }

    FragColor = vec4(result, diffuseColor.a); // Output the final color with alpha from the diffuse texture
}