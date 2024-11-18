#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

in Vertex{
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    vec3 diffuse = texture2D(diffuseTex, IN.texCoord).xyz;
    vec3 light = texture2D(diffuseLight, IN.texCoord).xyz;
    vec3 specular = texture2D(specularLight, IN.texCoord).xyz;

    fragColour.xyz = diffuse * 0.1; // Ambient
    fragColour.xyz += diffuse * light; // lambert diffuse
    fragColour.xyz += specular; // Specular
    fragColour.a = 1.0; 
}