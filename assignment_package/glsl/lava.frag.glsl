#version 150

in vec2 fs_UV;

out vec4 color;

uniform sampler2D u_RenderedTexture;
uniform ivec2 u_Dimensions;
uniform float u_Time;

float coeff = 0.156 * cos(u_Time / 360.f) + 0.643 * sin(u_Time / 180.f) +
           0.733 * cos(u_Time / 240.f) + 0.239 * sin(u_Time / 90.f);


// Worley noise implementation as per Noise Functions slide deck
// Modified by adding coefficient

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(112.3, 581.3)),
                          dot(p, vec2(213.4, 558.9))))
                 * 14423.3377 + coeff);
}

float worleyNoise(vec2 uv) {
    uv *= 12.5;
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = random2(uvInt + neighbor);
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }

    return minDist;
}

void main()
{
    // Use worley noise to displace the UV coordinates
    float minDistX1 = worleyNoise(fs_UV - vec2(1.f/u_Dimensions[0], 0));
    float minDistX2 = worleyNoise(fs_UV + vec2(1.f/u_Dimensions[0], 0));
    float minDistY1 = worleyNoise(fs_UV - vec2(0, 1.f/u_Dimensions[1]));
    float minDistY2 = worleyNoise(fs_UV + vec2(0, 1.f/u_Dimensions[1]));

    vec2 grad = vec2(minDistX2 - minDistX1, minDistY2 - minDistY1);

    vec4 textureColor = texture(u_RenderedTexture, grad + fs_UV);
    color = mix(textureColor, vec4(1.f, 0.f, 0.f, textureColor.a), 0.3);
}
