#version 330

uniform sampler2D screen_texture;
uniform float darken_screen_factor;
uniform float apply_vignette;
uniform float apply_fadeout;

in vec2 texcoord;
out vec4 color;

vec4 vignette(vec4 inputColor);
vec4 fadeout(vec4 inputColor);

// execute one of these shaders for every pixel on the screen
void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
    if (apply_vignette > 0.0) {
        in_color = vignette(in_color);
    }

    if (apply_fadeout > 0.0) {
        in_color = fadeout(in_color);
    }

    color = in_color;
}

vec4 vignette(vec4 inputColor)
{
    // https://www.youtube.com/watch?v=caQZKeAYgD8
    // vignette effect using distance from screen center
    // measure distance from this pixel in normalized coordinates (0 to 1) to center 
    // > note that vignette appears oval, because our screen is wider than it is tall; texture coordinates are not necessarily uniform between x and y dimensions
    // smoothstep for gradient vignette effect bounded within range of 0.3 to 0.8
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(texcoord, center);
    float vignette = smoothstep(0.3, 0.8, dist);

    // apply red tint based on vignette intensity (distance from center)
    vec4 red_tint = vec4(1.0, 0.0, 0.0, 1.0) * darken_screen_factor * vignette;

    // blend in scene and vignette overlay 
    return mix(inputColor, red_tint, darken_screen_factor * vignette);
}

vec4 fadeout(vec4 inputColor)
{
    vec4 fadeout_color = vec4(0.2, 0.2, 0.2, 1.0) * darken_screen_factor;
    return mix(inputColor, fadeout_color, darken_screen_factor);
}
