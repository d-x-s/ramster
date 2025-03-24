// textured.fs.glsl
#version 330 core
in vec2 UV;
out vec4 fragColor;

uniform sampler2D tex;
uniform vec2 camera_pos;
uniform float parallax_factor;
uniform vec2 texture_size;

void main() {
    // Scroll the UVs using camera position and parallax factor
    vec2 uv_offset = camera_pos * parallax_factor / texture_size;

    // Apply wrapping (GL_REPEAT handles overflow)
    vec2 final_uv = UV + uv_offset;

    fragColor = texture(tex, final_uv);
}
