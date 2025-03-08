#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

void AISystem::step(float elapsed_ms)
{

	// ENEMY AI
	/*
	DECISION TREE:

		1. Based on enemy type:
			
			1_a. OBSTACLE enemies: **NOTE: these enemies will not die or freeze after a collision.
			
				1a_a. Do not pursue the player. Based on movement area: **NOTE: if obstacle ends up outside of movement area, logic still applies so they'll end up inside again.
					
					1aa_a. If too close to left-hand-side, change direction to right-hand-side. 

					1aa_b. If too close to right-hand-side, change direction to left-hand-side.
					
					1aa_c. Keep moving in current direction.

			1_b. NON-OBSTACLE enemies:

				2b_a. COMMON enemies:

					2ba_a. If freeze-timer is above 0, DON'T MOVE.

					2ba_b. If player is to the left, move left.

					2ba_c. If player is to the right, move right.

				2b_b. SWARMING enemies:

					2bb_a. If freeze-timer is above 0, DON'T MOVE.

					2bb_b. IF TOO CLOSE TO THE GROUND, PULL UP!

					2bb_c. IF TOO CLOSE TO ANOTHER SWARMING ENEMY, move away from that enemy.

					2bb_d. Pursue the player. 
	*/

	// Box2D physics
	b2Vec2 nonjump_movement_force = { 0, 0 };
	b2Vec2 jump_impulse = { 0, 0 }; // not needed for now, here for future use
	const float forceMagnitude = ENEMY_GROUNDED_MOVEMENT_FORCE; 
	const float jumpImpulseMagnitude = ENEMY_JUMP_IMPULSE; // not needed for now, here for future use

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