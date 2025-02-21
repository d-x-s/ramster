#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

void AISystem::step(float elapsed_ms)
{

	// ENEMY AI
	/*
	For now, because these gooners lack the sheer steeze of our hamster, they'll be flightless.
	They do, however, have impeccable spidey senses and will be able to spot the hamster at all times.

	This means that they can only move horizontally and will always react, giving a pretty simple algo:
	
	IF HAMSTER IS LEFT:
	- MOVE LEFT
	IF HAMSTER IS RIGHT:
	- MOVE RIGHT
	ELSE:
	- DON'T MOVE
	
	This should be sufficient to have the enemies react to and move towards the hamster. 
	*/

	// Box2D physics
	b2Vec2 nonjump_movement_force = { 0, 0 };
	b2Vec2 jump_impulse = { 0, 0 }; // not needed for now, here for future use
	const float forceMagnitude = GROUNDED_MOVEMENT_FORCE; // might want to separate player vs enemy movement force....?
	const float jumpImpulseMagnitude = JUMP_IMPULSE; // not needed for now, here for future use

	// Get player and figure out player coords
	auto& player_registry = registry.players;
	Player& player = player_registry.components[0];
	Entity playerEntity = player_registry.entities[0];
	Motion& playerMotion = registry.motions.get(playerEntity);

	// Player position
	float player_posX = playerMotion.position[0];
	float player_posY = playerMotion.position[1]; // we wouldn't need this for now, here for future use.

	// Get enemy entities
	auto& enemy_registry = registry.enemies; //list of enemy entities stored in here

	// Iterate over each enemy and implement basic logic as commented above.
	for (int i = 0; i < enemy_registry.entities.size(); i++) {

		// Figure out the entity and position
		Entity enemyEntity = enemy_registry.entities[i];
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		// Enemy position
		float enemy_posX = enemyMotion.position[0];
		float enemy_posY = enemyMotion.position[1]; // we wouldn't need this for now, here for future use.


		// Decision tree. Simple for now, can expand into some monstrosity later.
		// Player is to the right.
		if (enemy_posX < player_posX) {
			// accelerate right
			nonjump_movement_force = { forceMagnitude, 0 };
		}
		// Player is to the left.
		else if (player_posX < enemy_posX) {
			// accelerate left
			nonjump_movement_force = { -forceMagnitude, 0 };
		}

		// Apply whatever decision made to box2D
		// sanity check that enemy entity decided to move before applying
		if (nonjump_movement_force != b2Vec2_zero || jump_impulse != b2Vec2_zero) {
			PhysicsBody& phys = registry.physicsBodies.get(enemyEntity);
			b2BodyId bodyId = phys.bodyId;
			float multiplier = 0.25f; // raising/lowering this number affects the speed of the enemy. lower = more sluggish.
			b2Vec2 bodyPosition = b2Body_GetPosition(bodyId);
			b2Body_ApplyForce(bodyId, nonjump_movement_force * multiplier, bodyPosition, true);
		}
		
	}

}