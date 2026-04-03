#version 330 core

#define MAX_LIGHTS 16

struct Light {
    vec3 position;
    vec4 color;
    float radius;
};

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform vec3 cameraPos;
uniform int lightCount;
uniform Light lights[MAX_LIGHTS];

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void){
    vec4 diffuse = texture(diffuseTex, IN.texCoord);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount; i++) {
        vec3 incident = normalize(lights[i].position - IN.worldPos);
        vec3 halfDir = normalize(incident + viewDir);
        float lambert = max(dot(incident, IN.normal), 0.0);
        float distance = length(lights[i].position - IN.worldPos);
        float attenuation = 1.0 - clamp(distance / lights[i].radius, 0.0, 1.0);
        float specFactor = pow(clamp(dot(halfDir, IN.normal), 0.0, 1.0), 60.0); // Shininess factor hardcoded for simplicity

        vec3 surface = diffuse.rgb * lights[i].color.rgb;
        result += surface * lambert * attenuation;;
        result += (lights[i].color.rgb * specFactor) * attenuation * 0.33; // Specular contribution scaled down for balance
    }

    // Directional light
    vec3 lightDir = normalize(vec3(-0.3, -1.0, -0.2));
    float diff = max(dot(IN.normal, -lightDir), 0.0);
    result += diffuse.rgb * vec3(1.0) * diff; // Directional light contribution
   
    result += diffuse.rgb * 0.05; // Ambient
    fragColour = vec4(result, diffuse.a);
}