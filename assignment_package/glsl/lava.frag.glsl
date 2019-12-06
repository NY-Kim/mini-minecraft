#version 150

in vec2 fs_UV;

out vec4 color;

uniform sampler2D u_RenderedTexture;
uniform ivec2 u_Dimensions;
uniform float u_Time;

float coeff = 0.156 * cos(u_Time / 360.f) + 0.643 * sin(u_Time / 180.f) +
           0.733 * cos(u_Time / 240.f) + 0.239 * sin(u_Time / 90.f);


// Perlin noise implementation as per Noise Functions slide deck
// Modified by adding coefficient

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(112.3, 581.3)),
                          dot(p, vec2(213.4, 558.9))))
                 * 14423.3377);
}

float surflet(vec2 p , vec2 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    vec2 diff = abs(p - gridPoint);
    vec2 t = vec2(1.f) - 6.f * pow(diff, vec2(5.f)) + 15.f * pow(diff, vec2(4.f)) - 10.f * pow(diff, vec2(3.f));

    // Get the random vector for the grid point (assume we wrote a function random2)
    vec2 gradient = random2(gridPoint);

    // Get the vector from the grid point to P
    vec2 diff2 = p - gridPoint;

    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff2, gradient);

    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y;

}

float worleyNoise(vec2 uv) {
    float surfletSum = 0.f;

    // Iterate over four integer corners surrounding uv
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, floor(uv) + vec2(dx, dy));
        }
    }

    return surfletSum;
}

void main()
{
    // Use worley noise to displace the UV coordinates
    vec2 grad = vec2(worleyNoise(fs_UV));
    vec4 textureColor = texture(u_RenderedTexture, coeff * grad + fs_UV);
    color = mix(textureColor, vec4(1.f, 0.f, 0.f, textureColor.a), 0.3f);
}
