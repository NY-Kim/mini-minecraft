#version 150

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    // TODO Homework 5
    vec4 textureColor = texture(u_RenderedTexture, fs_UV);
    float gray = 0.21 * textureColor[0] + 0.72 * textureColor[1] + 0.07 * textureColor[2];

    // Largest distance will be sqrt(0.5), so take that minus the actual distance to find modifier
    float distanceModifier = sqrt(0.5) - sqrt(pow((fs_UV[0] - 0.5) , 2) + pow(fs_UV[1] - 0.5, 2));

    float newColor = gray * distanceModifier;
    color = vec3(newColor, newColor, newColor);
}
