#version 150

in vec2 fs_UV;

out vec4 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    // TODO Homework 5
    vec4 textureColor = texture(u_RenderedTexture, fs_UV);
    color = mix(textureColor, vec4(1.f, 0.f, 0.f, textureColor.a), 0.3);
}
