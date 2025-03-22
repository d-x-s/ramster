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
					
					// IF WE HAVE HORIZONTAL MOVEMENT
					1aa_a. If too close to left-hand-side, reverse directions. 

					1aa_b. If too close to right-hand-side, reverse directions.
					
					// ONLY TRIGGERS IF WE DON'T HAVE HORIZONTAL MOVEMENT
					1aa_c. If too close to top, reverse directions.

					1aa_d. If too close to bottom, reverse directions.

					// Default triggers if obstacle not moving
					1aa_e. Keep moving in current direction.

			1_b. NON-OBSTACLE enemies:

				1b_a. If freeze-timer is above 0, decrement timer by elapsed time and exit.

					1ba_a. COMMON enemies:

						1baa_a. If player is to the left, move left.

						2baa_b. If player is to the right, move right.

					1ba_b. SWARMING enemies:

						[Excluded. This condition is effectively covered by 1bab_b.] 1bab_a. IF TOO CLOSE TO THE GROUND, PULL UP!

						1bab_a. Pursue the player.  **Note that this takes precedence. Collision avoidance is applied later.

						1bab_b. IF TOO CLOSE TO ANOTHER ENTITY THAT IS NOT THE PLAYER, move away from that entity.

						1bab_c. IF TOO FAR FROM SWARM, rejoin swarm.

	*/

	// Box2D physics
	b2Vec2 nonjump_movement_force = { 0, 0 };
	b2Vec2 jump_impulse = { 0, 0 }; // not needed for now, here for future use
	const float forceMagnitude = ENEMY_GROUNDED_MOVEMENT_FORCE; 

	// Different enemy types have different weights, so we'll need to apply some corrections here to compensate.

	const float swarmPursuit_forceMagnitude = forceMagnitude * 0.08;
	// The swarm applies corrections AFTER pursuing the player. To make sure player pursuit takes precedence, we'll make this force smaller so it corrects itself but will still
	// mainly pursue the player while doing so.
	const float swarmCorrection_forceMagnitude = swarmPursuit_forceMagnitude * 0.25; 

	const float obstacle_forceMagnitude = forceMagnitude * 200;

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

			// Player gets an immunity window after hitting obstacle.
			enemyComponent.freeze_time -= elapsed_ms;

			// figure out lower and upper x-bound of patrol range (y doesn't matter as our movement vector ensures that if x triggers, y also triggers)
			float left_hand_side = min(enemyComponent.movement_area_point_a.x, enemyComponent.movement_area_point_b.x);
			float right_hand_side = max(enemyComponent.movement_area_point_a.x, enemyComponent.movement_area_point_b.x);
			float bottom = min(enemyComponent.movement_area_point_a.y, enemyComponent.movement_area_point_b.y);
			float top = max(enemyComponent.movement_area_point_a.y, enemyComponent.movement_area_point_b.y);


			// compute the vector
			vec2 point_a = enemyComponent.movement_area_point_a;
			vec2 point_b = enemyComponent.movement_area_point_b;
			// get deltas
			float delta_x = point_b.x - point_a.x;
			float delta_y = point_b.y - point_a.y;

			// normalize on x-axis
			if (delta_x != 0 && delta_y != 0) {
				delta_y = delta_y / delta_x;
				delta_x = delta_x / delta_x; //could just set this to 1?
			}
			else if (delta_y == 0) {
				delta_x = 1;
			}
			else if (delta_x == 0) {
				delta_y = 1;
			}

			// normalize to always point right, or up if x = 0.
			if (delta_x < 0) {
				delta_x *= -1;
				delta_y *= -1;
			}
			if (delta_x == 0 && delta_y < 0) {
				delta_y *= -1;
			}


			// Decision tree here
			// Only when we have a delta-x
			if (delta_x != 0) {
				if (enemyMotion.position.x <= left_hand_side + GRID_CELL_WIDTH_PX / 2) {
					// 1aa_a. If too close to LHS, reverse directions.

					// accelerate towards top-right
					nonjump_movement_force = { delta_x * obstacle_forceMagnitude, delta_y * obstacle_forceMagnitude };
				}
				else if (enemyMotion.position.x >= right_hand_side - GRID_CELL_WIDTH_PX / 2) {
					// 1aa_b. If too close to RHS, reverse directions.

					// accelerate towards bottom-left
					nonjump_movement_force = { -delta_x * obstacle_forceMagnitude, -delta_y * obstacle_forceMagnitude };
				}
			}
			// If vertical movement then we switch logic to y-axis
			else if (delta_x == 0) {
				if (enemyMotion.position.y <= bottom + GRID_CELL_HEIGHT_PX / 2) {
					// 1aa_d. If too close to bottom, reverse directions.

					// accelerate towards top-right
					nonjump_movement_force = { delta_x * obstacle_forceMagnitude, delta_y * obstacle_forceMagnitude };
				}
				else if (enemyMotion.position.y >= top - GRID_CELL_HEIGHT_PX / 2) {
					// 1aa_c. If too close to top, reverse directions.

					// accelerate towards bottom-left
					nonjump_movement_force = { -delta_x * obstacle_forceMagnitude, -delta_y * obstacle_forceMagnitude };
				}
			}
			else {
				// 1aa_c. Keep moving in current direction.
				if (enemy_velocity.x == 0) {
					// just move in default RHS/UP direction.
					nonjump_movement_force = { delta_x * obstacle_forceMagnitude * 100, delta_y * obstacle_forceMagnitude * 100 };
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
					vec2 entityToAvoid = vec2(0, 0); // This will get modifed after calling helper function to be the position of the enemy to avoid
					vec2 swarmRejoinLocation = vec2(-1000, -1000); // This will get modified after calling helper function to be the position of the swarm to rejoin

					// Reset whatever force they had
					nonjump_movement_force = { 0, 0 };

					if (true) {
						// 1bab_c. Pursue the player.

						// Apply impulse on both X and Y axis to pursue player
						if (player_posX < enemy_posX) {
							// If player is to the left, move left.

							// accelerate left
							nonjump_movement_force += { -swarmPursuit_forceMagnitude, nonjump_movement_force.y };
						}
						else {
							// If player is to the right, move right.

							// accelerate right
							nonjump_movement_force += { swarmPursuit_forceMagnitude, nonjump_movement_force.y };
						}
						if (player_posY <= enemy_posY) {
							// If player is below, go down
							nonjump_movement_force += { nonjump_movement_force.x, -swarmPursuit_forceMagnitude };
						}
						else {
							// If player is above, go up
							nonjump_movement_force += { nonjump_movement_force.x, swarmPursuit_forceMagnitude };
						}
					}

					if (tooFarFromSwarm(enemyEntity, swarmRejoinLocation)) {
					// 1bab_c. IF TOO FAR FROM SWARM, rejoin swarm.

						// Apply impulse on both X and Y axis to pursue closest swarm enemy
						if (swarmRejoinLocation.x < enemy_posX) {
							// If closest swarm is to the left, move left.

							// accelerate left
							nonjump_movement_force += { -swarmCorrection_forceMagnitude, nonjump_movement_force.y };
						}
						else {
							// If closest swarm is to the right, move right.

							// accelerate right
							nonjump_movement_force += { swarmCorrection_forceMagnitude, nonjump_movement_force.y };
						}
						if (swarmRejoinLocation.y <= enemy_posY) {
							// If closest swarm is below, go down
							nonjump_movement_force += { nonjump_movement_force.x, -swarmCorrection_forceMagnitude };
						}
						else {
							// If closest swarm above, go up
							nonjump_movement_force += { nonjump_movement_force.x, swarmCorrection_forceMagnitude };
						}
					}

					else if (tooCloseToSwarm(enemyEntity, entityToAvoid)) {
						// 1bab_b. IF TOO CLOSE TO ANOTHER SWARMING ENTITY THAT IS NOT THE PLAYER, move away from that entity.
						
						// We will apply both an x and y impulse so it goes in the opposite direction.
						if (entityToAvoid.x <= enemyMotion.position.x) {
							// Need to go right
							nonjump_movement_force += { swarmCorrection_forceMagnitude, nonjump_movement_force.y }; 
						}
						else {
							// Need to go left
							nonjump_movement_force += { -swarmCorrection_forceMagnitude, nonjump_movement_force.y };
						}
						if (entityToAvoid.y <= enemyMotion.position.y) {
							// Need to go up
							nonjump_movement_force += { nonjump_movement_force.x, swarmCorrection_forceMagnitude };
						}
						else {
							// Need to go down
							nonjump_movement_force += { nonjump_movement_force.x, -swarmCorrection_forceMagnitude };
						}
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
bool AISystem::tooCloseToSwarm(Entity swarmEnemy, vec2& entityToAvoid)
{
	// Stuff that we'll need
	Motion& enemyMotion = registry.motions.get(swarmEnemy);
	Enemy& enemyComponent = registry.enemies.get(swarmEnemy);


	// Ground check
	if (enemyMotion.position.y <= GRID_CELL_HEIGHT_PX / 4) {
		entityToAvoid = vec2(enemyMotion.position.x, 0);
		return true;
	}

	// Now we just iterate over every physics body entity to see if it's too close
	std::vector<Entity> physicsEntities = registry.physicsBodies.entities;
	for (int i = 0; i < physicsEntities.size(); i++) {
		Entity entity = physicsEntities[i];

		// Ensure that we're not dealing with the player or the swarm enemy itself and the enemy has a motion component
		if ((!registry.players.has(entity)) && (!(entity == swarmEnemy)) && registry.motions.has(entity)) {
			Motion entityMotion = registry.motions.get(entity);

			if (abs(enemyMotion.position.x - entityMotion.position.x) <= GRID_CELL_WIDTH_PX/4 || abs(enemyMotion.position.y - entityMotion.position.y) <= GRID_CELL_HEIGHT_PX/4) {
				entityToAvoid = entityMotion.position;
				return true;
			}
		}
	}

	// If no enemies are too close then we can go ahead and return false
	return false;
}

bool AISystem::tooFarFromSwarm(Entity swarmEnemy, vec2& closestSwarm) {

	// Safety check: if there's only a single swarm enemy return false.
	

	// Stuff that we'll need
	Motion& selfMotion = registry.motions.get(swarmEnemy);
	Enemy& selfComponent = registry.enemies.get(swarmEnemy);

	// Now we just iterate over every enemy entity to see if we're too far from the swarm
	std::vector<Entity> enemyEntities = registry.enemies.entities;
	for (int i = 0; i < enemyEntities.size(); i++) {
		Entity entity = enemyEntities[i];
		Enemy enemyComponent = registry.enemies.get(entity);

		// Ensure that we're dealing with a swarm enemy, but not the swarm enemy itself
		if ((enemyComponent.enemyType == SWARM) && (!(entity == swarmEnemy))) {
			Motion entityMotion = registry.motions.get(entity);
			float currClosestDist = (selfMotion.position.x - closestSwarm.x) * (selfMotion.position.x - closestSwarm.x)
				+ (selfMotion.position.y - closestSwarm.y) * (selfMotion.position.y - closestSwarm.y); //Pythagorean distance
			float newClosestDist = (selfMotion.position.x - entityMotion.position.x) * (selfMotion.position.x - entityMotion.position.x)
				+ (selfMotion.position.y - entityMotion.position.y) * (selfMotion.position.y - entityMotion.position.y);

			// If this swarm enemy is closer, we mark its location as the closest
			if (newClosestDist < currClosestDist) {
				closestSwarm = entityMotion.position;
			}

		}
	}

	// if closest swarm enemy is still too far, then then we'll return true.
	return (abs(closestSwarm.x - selfMotion.position.x) > SWARM_ENEMY_PROXIMITY || abs(closestSwarm.y - selfMotion.position.y) > SWARM_ENEMY_PROXIMITY);
}


