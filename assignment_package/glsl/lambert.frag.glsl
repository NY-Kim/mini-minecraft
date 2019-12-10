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
