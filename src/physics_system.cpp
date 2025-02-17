// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Constructor
PhysicsSystem::PhysicsSystem(b2WorldId worldId) : worldId(worldId) {}

// Destructor
PhysicsSystem::~PhysicsSystem() {}

// Advances physics simulation
void PhysicsSystem::step(float elapsed_ms)
{
    // Box2D v3 Upgrade: Use `b2World_Step()` instead of `world.Step()`
    float timeStep = elapsed_ms / 1000.0f;
    b2World_Step(worldId, timeStep, 4);  // 4 is the recommended substep count

    // Access player registry
    auto& player_registry = registry.players;
    Player& player = player_registry.components[0];

    // Access physics body registry
    auto& physicsBody_registry = registry.physicsBodies;
    Entity entity_physicsBody = player_registry.entities[0];

    PhysicsBody& component_physicsBody = registry.physicsBodies.get(entity_physicsBody);
    b2BodyId bodyId = component_physicsBody.bodyId;
    b2Vec2 position = b2Body_GetPosition(bodyId);

    // Update motion component
    Motion& component_motion = registry.motions.get(entity_physicsBody);
    component_motion.position = vec2(position.x, position.y);

    // Update camera position
    Camera& camera = registry.cameras.get(entity_physicsBody);
    camera.position = vec2(position.x, position.y);

    // Debugging output
    // std::cout << "Box2D Ball Body position = (" << position.x << ", " << position.y << ")\n";
}
