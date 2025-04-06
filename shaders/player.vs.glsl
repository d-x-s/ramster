#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform bool u_flipTextureX; // <-- Added flip control

void main()
{
	// Flip texcoord horizontally if needed
	texcoord = vec2(u_flipTextureX ? (1.0 - in_texcoord.x) : in_texcoord.x, in_texcoord.y);

	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
