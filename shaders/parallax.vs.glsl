// textured.vs.glsl
#version 330 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

uniform mat3 transform;
uniform mat3 projection;

out vec2 UV;

void main() {
    vec3 world_pos = transform * vec3(in_position.xy, 1.0);
    gl_Position = vec4((projection * world_pos).xy, 0.0, 1.0);

    UV = in_texcoord;
}
