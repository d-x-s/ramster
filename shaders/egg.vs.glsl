#version 330

// Input attributes
in vec3 in_color;		// the color for each vertex
in vec3 in_position;	// the vertex position in object/local space

out vec3 vcolor;		// output variable passed to the fragment shader

// Application data (uniforms are constant during draw call and are set by the program)
uniform mat3 transform;	// uniform for transforming vertices (scaling, rotating, translating)
uniform mat3 projection;// uniform that projects vertex positions into clip space

void main()
{
	// directly pass vertex color to fragment shader
	vcolor = in_color;	

	// computes the transformed position of the vertex
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);

	// gl_position is a built-in variable representing the final position of the vertex in clip space
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}