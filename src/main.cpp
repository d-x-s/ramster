
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
    // Setup Box2D world
    b2Vec2 gravity(0.0f, -98.0f);
    b2World world(gravity);

	// Room dimensions
	const float roomWidth = 840.0f;
	const float roomHeight = 600.0f;
	const float wallThickness = 20.0f; // half-width for SetAsBox

	// Floor
	b2BodyDef floorDef;
	floorDef.position.Set(roomWidth / 2, 0.0f);
	b2Body* floorBody = world.CreateBody(&floorDef);
	b2PolygonShape floorShape;
	floorShape.SetAsBox(roomWidth / 2, wallThickness);
	floorBody->CreateFixture(&floorShape, 0.0f);

	// Ceiling
	b2BodyDef ceilingDef;
	ceilingDef.position.Set(roomWidth / 2, roomHeight);
	b2Body* ceilingBody = world.CreateBody(&ceilingDef);
	b2PolygonShape ceilingShape;
	ceilingShape.SetAsBox(roomWidth / 2, wallThickness);
	ceilingBody->CreateFixture(&ceilingShape, 0.0f);

	// Left wall
	b2BodyDef leftWallDef;
	leftWallDef.position.Set(0.0f, roomHeight / 2);
	b2Body* leftWallBody = world.CreateBody(&leftWallDef);
	b2PolygonShape leftWallShape;
	leftWallShape.SetAsBox(wallThickness, roomHeight / 2);
	leftWallBody->CreateFixture(&leftWallShape, 0.0f);

	// Right wall
	b2BodyDef rightWallDef;
	rightWallDef.position.Set(roomWidth, roomHeight / 2);
	b2Body* rightWallBody = world.CreateBody(&rightWallDef);
	b2PolygonShape rightWallShape;
	rightWallShape.SetAsBox(wallThickness, roomHeight / 2);
	rightWallBody->CreateFixture(&rightWallShape, 0.0f);

	// global systems
	WorldSystem   world_system(world);
    PhysicsSystem physics_system(world);
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
