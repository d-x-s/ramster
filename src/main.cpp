
#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <box2d/box2d.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Setup Box2D v3 world
	float lengthUnitsPerMeter = 100.0f;
	b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity.y = -9.8f;
	b2WorldId worldId = b2CreateWorld(&worldDef);

	b2Vec2 gravity_vector;
	gravity_vector.x = 0.f;
	gravity_vector.y = GRAVITY;

	b2World_SetGravity(worldId, gravity_vector);

	// Room dimensions
	const float roomWidth = WINDOW_WIDTH_PX * 3.0;
	const float roomHeight = WINDOW_HEIGHT_PX;
	const float wallThickness = 20.0f; // half-width for SetAsBox

	auto create_wall = [&](float x, float y, float halfWidth, float halfHeight) {
		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.position = b2Vec2{ x, y };

		b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

		b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

		shapeDef.friction = 0.1f;

		return bodyId;
	};

	// Create room boundaries
	b2BodyId floorId = create_wall(roomWidth / 2, 0.0f, roomWidth / 2, wallThickness);			// Floor
	b2BodyId ceilingId = create_wall(roomWidth / 2, roomHeight, roomWidth / 2, wallThickness);	// Ceiling
	b2BodyId leftWallId = create_wall(0.0f, roomHeight / 2, wallThickness, roomHeight / 2);		// Left Wall
	b2BodyId rightWallId = create_wall(roomWidth, roomHeight / 2, wallThickness, roomHeight / 2);	// Right Wall

	// global systems
	WorldSystem   world_system(worldId);
    PhysicsSystem physics_system(worldId);
    AISystem	  ai_system;
	RenderSystem  renderer_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		bool game_active = world_system.step(elapsed_ms);
		if (game_active) {
			ai_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			world_system.handle_collisions();
		};

		renderer_system.draw(elapsed_ms, game_active);
	}

	return EXIT_SUCCESS;
}
