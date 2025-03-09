#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

void AISystem::step(float elapsed_ms)
{

	// ENEMY AI
	// DECISION TREE:
	/*
	DECISION TREE:

		1. Based on enemy type:
			
			1_a. OBSTACLE enemies: **NOTE: these enemies will not die or freeze after a collision.
			
				1a_a. Do not pursue the player. Based on movement area: **NOTE: if obstacle ends up outside of movement area, logic still applies so they'll end up inside again.
					
					1aa_a. If too close to left-hand-side, change direction to right-hand-side. 

					1aa_b. If too close to right-hand-side, change direction to left-hand-side.
					
					1aa_c. Keep moving in current direction.

			1_b. NON-OBSTACLE enemies:

				1b_a. If freeze-timer is above 0, decrement timer by elapsed time and exit.

					1ba_a. COMMON enemies:

						1baa_a. If player is to the left, move left.

						2baa_b. If player is to the right, move right.

					1ba_b. SWARMING enemies:

						[Excluded. This condition is effectively covered by 1bab_b.] 1bab_a. IF TOO CLOSE TO THE GROUND, PULL UP!

						1bab_b. IF TOO CLOSE TO ANOTHER ENTITY THAT IS NOT THE PLAYER, move away from that entity.

						1bab_c. Pursue the player. 
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

		// Figure out enemy details
		Entity enemyEntity = enemy_registry.entities[i];
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		Enemy& enemyComponent = registry.enemies.get(enemyEntity);

		// Get Box2D Speed
		b2BodyId enemy_id = registry.physicsBodies.get(enemyEntity).bodyId;
		b2Vec2 enemy_velocity = b2Body_GetLinearVelocity(enemy_id);

		// Enemy position
		float enemy_posX = enemyMotion.position[0];
		float enemy_posY = enemyMotion.position[1]; // we wouldn't need this for now, here for future use.


		// DECISION TREE

		// 1. Figure out enemy type.
		if (enemyComponent.enemyType == OBSTACLE) {
			// 1_a.OBSTACLE enemies : **NOTE : these enemies will not die or freeze after a collision.

			if (enemyMotion.position.x <= enemyComponent.movement_area[0] + GRID_CELL_WIDTH_PX/2) {
				// 1aa_a. If too close to left-hand-side, change direction to right-hand-side.

				// accelerate right
				nonjump_movement_force = { forceMagnitude * 100, 0 };
			}
			else if (enemyMotion.position.x >= enemyComponent.movement_area[1] - GRID_CELL_WIDTH_PX/2) {
				// 1aa_b. If too close to right-hand-side, change direction to left-hand-side.

				// accelerate left
				nonjump_movement_force = { -forceMagnitude * 100, 0 };
			}
			else {
				// 1aa_c. Keep moving in current direction.
				if (enemy_velocity.x == 0) {
					nonjump_movement_force = { forceMagnitude * 10000, 0 };
				}
			}

		}
		else {
			// 1_b. NON-OBSTACLE enemies:
			if (enemyComponent.freeze_time > 0) {
				// 1b_a. If freeze-timer is above 0, decrement timer by elapsed time and exit.
				enemyComponent.freeze_time -= elapsed_ms;

			}
			else {
				if (enemyComponent.enemyType == COMMON) {
					// 1ba_a.COMMON enemies :

					if (player_posX < enemy_posX) {
						// 1baa_a. If player is to the left, move left.

						// accelerate left
						nonjump_movement_force = { -forceMagnitude, 0 };
					}
					else if (enemy_posX < player_posX) {
						// 2baa_b. If player is to the right, move right.

						// accelerate right
						nonjump_movement_force = { forceMagnitude, 0 };
					}
					
				}
				else if (enemyComponent.enemyType == SWARM) {
					// 1ba_b. SWARMING enemies:

					if (tooCloseToSwarm(enemyEntity)) {
						// 1bab_b. IF TOO CLOSE TO ANOTHER SWARMING ENTITY THAT IS NOT THE PLAYER, move away from that entity.

					}
					else {
						// 1bab_c. Pursue the player.

					}

				}
			}
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

// Helper function that will determine if a swarm enemy is too close to another entity (that is not the player).
bool AISystem::tooCloseToSwarm(Entity swarmEnemy)
{
	return false;
}


