#version 330 core

uniform sampler2D diffuseTex;
uniform sampler1D rainbowGradientTex;
uniform sampler2D dudvMap;
uniform float moveFactor;
uniform float timeOffset;
uniform int useRainbowGradient;

in Vertex{
    vec2 texCoord;
} IN;

out vec4 fragColor;

void main(){
    vec4 baseColor = texture(diffuseTex, IN.texCoord);

    if (useRainbowGradient == 0) {
        fragColor = baseColor;
        return;
    }
    else if (useRainbowGradient == 1) {
        // Sample DuDv for distortion
        vec2 distortion = texture(dudvMap, IN.texCoord + vec2(moveFactor * 0.05)).rg;
        distortion = (distortion - 0.5) * 2.0; // Convert from [0,1] to [-1,1]

        // Use distorted coords + time to index into gradient
        float colorIndex = fract(IN.texCoord.x + distortion.x * 0.1 + timeOffset * 0.3);
        vec3 rainbowColor = texture(rainbowGradientTex, colorIndex).rgb;

        fragColor = vec4(rainbowColor, 1.0);
    }
}