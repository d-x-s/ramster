// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

PhysicsSystem::PhysicsSystem(b2World& world_ref) : world(world_ref) { }
PhysicsSystem::~PhysicsSystem() { }

void PhysicsSystem::step(float elapsed_ms)
{
	world.Step(1.0f / 60.0f, 6, 2); // Box2D: step the world

	auto& player_registry = registry.players;
	Player& player = player_registry.components[0];

	auto& physicsBody_registry = registry.physicsBodies;
	Entity entity_physicsBody = player_registry.entities[0];

	PhysicsBody& component_physicsBody = registry.physicsBodies.get(entity_physicsBody);
	b2Body* body = component_physicsBody.body;
	b2Vec2 position = body->GetPosition();
	Motion& component_motion = registry.motions.get(entity_physicsBody);
	component_motion.position = vec2(position.x, position.y);

	std::cout << "Box2D Ball Body position = (" << position.x << ", " << position.y << ")\n";
}