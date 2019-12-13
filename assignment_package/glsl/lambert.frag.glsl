#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture;
uniform float u_Time;
uniform vec4 u_Player;
// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_LightVec;

in vec4 fs_Nor;
in vec4 fs_Col;
in vec4 fs_Pos;
in vec2 fs_UV;
in float fs_CosPow;
in float fs_AniFlag;

in vec4 fs_CameraPos;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

float rand(vec2 n) {
        return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
        const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
        return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float fbm(vec2 x) {
        float v = 0.0;
        float a = 0.5;
        vec2 shift = vec2(100);
        // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
        for (int i = 0; i < 5; ++i) {
                v += a * noise(x);
                x = rot * x * 2.0 + shift;
                a *= 0.5;
        }
        return v;
}

void main()
{
    vec2 uv = fs_UV;
    if (fs_AniFlag != 0.0f) {
        if (fs_Nor[0] != 0.0f || fs_Nor[2] != 0.0f) {
            uv = vec2(fs_UV.x, fs_UV.y + mod(u_Time / 8000.f, 1.f / 16.f));
        } else {
            uv = vec2(fs_UV.x + mod(u_Time / 8000.f, 1.f / 16.f), fs_UV.y);
        }
    }

    vec4 diffuse_color = texture(u_Texture, uv);

    if (fs_UV.x >= 8.f / 16.f && fs_UV.y >= 13.f / 16.f && fs_UV.x <= 9.f / 16.f && fs_UV.y <= 14.f / 16.f) {
        float n = fbm(fs_Pos.xz / 16.f);
        vec3 color = mix(vec3(0.5, 0.5, 0.25) * diffuse_color.rgb, diffuse_color.rgb, n);
        diffuse_color = vec4(color, diffuse_color.a);
    }

    if (fs_UV.x >= 2.f / 16.f && fs_UV.y >= 11.f / 16.f && fs_UV.x <= 3.f / 16.f && fs_UV.y <= 12.f / 16.f) {
        float n = fbm(fs_Pos.xz / 16.f);
        vec3 color = mix(vec3(0.7, 0.7, 0.7) * fs_Pos.y / 150.f , diffuse_color.rgb, n);
        diffuse_color = vec4(color, diffuse_color.a);
    }

    float diffuse_term = dot(normalize(fs_Nor), normalize(fs_LightVec));
    diffuse_term = clamp(diffuse_term, 0, 1);
    float ambient_term = 0.2f;

    vec4 view_vector = fs_CameraPos - fs_Pos;
    vec4 light_vector = fs_LightVec;
    vec4 halfway_vector = normalize((view_vector + light_vector) / 2);

    vec4 surface_normal = normalize(fs_Nor);
    float specular_intensity = max(pow(dot(halfway_vector, surface_normal), fs_CosPow), 0);

    float light_intensity = diffuse_term + ambient_term + specular_intensity;

    vec4 color = vec4(diffuse_color.rgb * light_intensity, diffuse_color.a);
    float a = diffuse_color.a;
    vec4 sky_color = vec4(0.37f, 0.74f, 1.0f, 1);
    color = mix(color, sky_color, pow(smoothstep(0, 1, min(1.0, length(fs_Pos.xz - u_Player.xz) * 0.01)), 2.0f));

    out_Col = vec4(color.rgb, a);
}
