#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float translucent_alpha; // <-- transparency control

// Output color
layout(location = 0) out vec4 color;

void main()
{
    color = vec4(fcolor, translucent_alpha) * texture(sampler0, texcoord);
}
