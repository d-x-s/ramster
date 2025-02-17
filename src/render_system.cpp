
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

void RenderSystem::drawGridLine(Entity entity, const mat3& projection) {
	GridLine& gridLine = registry.gridLines.get(entity);
	Transform transform;

	/* Note(@Davis):
	* The provided code was treating end_pos as a scale factor rather than the
	* actual endpoint. 
	* If you recall from 314, "scaling" an object stretches it relative to the
	* center of the object, in both directions. So vertically scaling a circle for example,
	* makes it into an oval relative to its local origin.
	*  
	* After adding a camera that moved with the player, it was quite obvious that
	* the provided code simply drew a line of 2x the window width/height, because
	* I could see half of the grid line stretching past the world boundaries.
	* That is, half of each line contributed to forming the grid.
	* Then, the other half stretched out into empty space.
	* Of course we wouldn't notice because the initial code had a static camera.
	* This means that end_pos was misrepresented (kind of a bug...?)!
	*  
	* My changes move the line to its "midpoint", then scale the line to the intended
	* size. In other words, start_pos and end_pos represent the actual start and ending
	* positions of the line. Now, grid lines stay within the world boundary.
	*  
	* transform.translate(gridLine.start_pos);
	* transform.scale(gridLine.end_pos);
	*/
	transform.translate((gridLine.start_pos + gridLine.end_pos) * 0.5f);  // Move to the midpoint
	transform.scale(abs(gridLine.end_pos - gridLine.start_pos));		  // Scale based on the actual size

	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		GLint in_color_loc    = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	// CK: std::cout << "line color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection, float elapsed_ms, bool game_active)
{
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	// Transform --> Translate --> Scale --> Rotate
	
	Motion& motion = registry.motions.get(entity);
	Transform transform;

	// Get the actual physics body from registry
	if (registry.physicsBodies.has(entity)) {
		PhysicsBody& phys = registry.physicsBodies.get(entity);

		// Get body's world position (correct physics position)
		// b2Vec2 p = b2Body_GetWorldPoint(phys.bodyId, { 0.0f, 0.0f });
		// motion.position = vec2(p.x, p.y); // Sync motion position with physics

		// Get body's rotation
		b2Rot rotation = b2Body_GetRotation(phys.bodyId);
		float angleRadians = b2Rot_GetAngle(rotation);
		motion.angle = glm::degrees(angleRadians); // Convert radians to degrees

		// Report data
		std::cout << "Box2D Ball Body position = (" << b2Body_GetPosition(phys.bodyId).x << ", " << b2Body_GetPosition(phys.bodyId).y << ")\n";
	}

	// TRANSLATE: Move to the correct position
	transform.translate(motion.position);

	// SCALE
	transform.scale(motion.scale + 10.0f);

	// ROTATE: Apply Box2D rotation to sprite
	transform.rotate(glm::radians(motion.angle));

	// SCALE
	// apply custom scale to each animation frame if scale data is embedded
	//if (!render_request.animation_frames_scale.empty()) {
	//	if (!render_request.animation_frames_scale.empty()) {
	//		int index = (render_request.animation_current_frame + 1) % render_request.animation_frames.size();
	//		transform.scale(motion.scale * render_request.animation_frames_scale[index]);
	//	}
	//}
	//else { // otherwise juset set to the static size
	//	transform.scale(motion.scale);
	//}

	// ROTATE
	// transform.rotate(radians(motion.angle));

	// handle animation if this render request has animation data embedded
	assert(registry.renderRequests.has(entity));
	RenderRequest& render_request = registry.renderRequests.get(entity);
	if (!render_request.animation_frames.empty() && game_active) {
		render_request.animation_elapsed_time += elapsed_ms;

		// if this is not a looping animation and it is already complete, remove it
		if (!render_request.is_loop &&
			render_request.animation_current_frame >= render_request.animation_frames.size()) {
			registry.remove_all_components_of(entity);
			return;
		}

		// if enough time has passed, switch frames
		if (render_request.animation_elapsed_time >= render_request.animation_frame_time) {
			render_request.animation_elapsed_time = 0;
			render_request.animation_current_frame += 1;
			int access_index = render_request.animation_current_frame % render_request.animation_frames.size();
			render_request.used_texture = render_request.animation_frames[access_index];
		}
	}

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	// .obj entities
	else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the vignette texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// add the "vignette" effect
	const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];

	// set clock
	GLuint time_uloc       = glGetUniformLocation(vignette_program, "time");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));

	// screen effects
	GLuint dead_timer_uloc = glGetUniformLocation(vignette_program, "darken_screen_factor");
	GLuint apply_vignette_uloc = glGetUniformLocation(vignette_program, "apply_vignette");
	GLuint apply_fadeout_uloc = glGetUniformLocation(vignette_program, "apply_fadeout");

	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	// std::cout << "screen.darken_screen_factor: " << screen.darken_screen_factor << " entity id: " << screen_state_entity << std::endl;
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	glUniform1f(apply_vignette_uloc, screen.vignette);
	glUniform1f(apply_fadeout_uloc, screen.fadeout);
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(float elapsed_ms, bool game_active)
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	
	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	
	// white background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();

	// draw all entities with a render request to the frame buffer
	for (Entity entity : registry.renderRequests.entities)
	{
		// filter to entities that have a motion component
		if (registry.motions.has(entity)) {
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D, elapsed_ms, game_active);
		}
		// draw grid lines separately, as they do not have motion but need to be rendered
		else if (registry.gridLines.has(entity)) {
			drawGridLine(entity, projection_2D);
		}
	}

	// draw framebuffer to screen
	// adding "vignette" effect when applied
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

/* Note(@Davis):
* Although we store the position of the camera, a moving camera doesn't actually "move".
* The display technically remains static, but view of the world is being shifted.
* In other words, world coordinates are being offset such that the player is always center.
*/
mat3 RenderSystem::createProjectionMatrix() {
	Camera camera = registry.cameras.components[0];

	// Compute relative viewport bounds
	float left = camera.position.x - WINDOW_WIDTH_PX / 2.0f;
	float bottom = camera.position.y - WINDOW_HEIGHT_PX / 2.0f;
	float right = left + WINDOW_WIDTH_PX;
	float top = bottom + WINDOW_HEIGHT_PX;

	// Scale factors, to scale to [-1, 1] OpenGl coordinate space
	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);

	// Translation factors, shifts coordinates relative to the camera and by extension the player
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{ sx, 0.f, 0.f },
		{ 0.f, sy, 0.f },
		{ tx, ty, 1.f }
	};
}