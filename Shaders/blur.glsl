#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D image;
uniform bool horizontal;

void main()
{
    float weight[5] = float[]( // Gaussian weights for a 5x5 kernel
        0.227027,
        0.1945946,
        0.1216216,
        0.054054,
        0.016216
    );

    vec2 tex_offset = 1.0 / textureSize(image, 0); // Size of a single texel
    vec3 result = texture(image, vTexCoord).rgb * weight[0]; // Center pixel

    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i]; // Right side
            result += texture(image, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i]; // Left side
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i]; // Down
            result += texture(image, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i]; // Up
        }
    }
    FragColor = vec4(result, 1.0);
}