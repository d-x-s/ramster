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

						1bab_c. IF TOO FAR FROM SWARM, rejoin swarm.

						1bab_d. Pursue the player. 
	*/

	// Box2D physics
	b2Vec2 nonjump_movement_force = { 0, 0 };
	b2Vec2 jump_impulse = { 0, 0 }; // not needed for now, here for future use
	const float forceMagnitude = ENEMY_GROUNDED_MOVEMENT_FORCE; 
	const float swarm_forceMagnitude = forceMagnitude * 0.1;
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
					vec2 entityToAvoid = vec2(0, 0); // This will get modifed after calling helper function to be the position of the enemy to avoid
					vec2 swarmRejoinLocation = vec2(-1000, -1000); // This will get modified after calling helper function to be the position of the swarm to rejoin
					if (tooFarFromSwarm(enemyEntity, swarmRejoinLocation)) {
					// 1bab_c. IF TOO FAR FROM SWARM, rejoin swarm.

						// Apply impulse on both X and Y axis to pursue closest swarm enemy
						if (swarmRejoinLocation.x < enemy_posX) {
							// If closest swarm is to the left, move left.

							// accelerate left
							nonjump_movement_force = { -swarm_forceMagnitude, nonjump_movement_force.y };
						}
						else {
							// If closest swarm is to the right, move right.

							// accelerate right
							nonjump_movement_force = { swarm_forceMagnitude, nonjump_movement_force.y };
						}
						if (swarmRejoinLocation.y <= enemy_posY) {
							// If closest swarm is below, go down
							nonjump_movement_force = { nonjump_movement_force.x, -swarm_forceMagnitude };
						}
						else {
							// If closest swarm above, go up
							nonjump_movement_force = { nonjump_movement_force.x, swarm_forceMagnitude * 5 };
						}
					}
					else if (tooCloseToSwarm(enemyEntity, entityToAvoid)) {
						// 1bab_b. IF TOO CLOSE TO ANOTHER SWARMING ENTITY THAT IS NOT THE PLAYER, move away from that entity.
						
						// We will apply both an x and y impulse so it goes in the opposite direction.
						if (entityToAvoid.x <= enemyMotion.position.x) {
							// Need to go right
							nonjump_movement_force = { swarm_forceMagnitude, nonjump_movement_force.y }; 
						}
						else {
							// Need to go left
							nonjump_movement_force = { -swarm_forceMagnitude, nonjump_movement_force.y };
						}
						if (entityToAvoid.y <= enemyMotion.position.y) {
							// Need to go up
							nonjump_movement_force = { nonjump_movement_force.x, swarm_forceMagnitude * 5 };
						}
						else {
							// Need to go down
							nonjump_movement_force = { nonjump_movement_force.x, -swarm_forceMagnitude };
						}
					}
					
					else {
						// 1bab_c. Pursue the player.

						// Apply impulse on both X and Y axis to pursue player
						if (player_posX < enemy_posX) {
							// If player is to the left, move left.

							// accelerate left
							nonjump_movement_force = { -swarm_forceMagnitude, nonjump_movement_force.y };
						}
						else {
							// If player is to the right, move right.

							// accelerate right
							nonjump_movement_force = { swarm_forceMagnitude, nonjump_movement_force.y };
						}
						if (player_posY <= enemy_posY) {
							// If player is below, go down
							nonjump_movement_force = { nonjump_movement_force.x, -swarm_forceMagnitude };
						}
						else {
							// If player is above, go up
							nonjump_movement_force = { nonjump_movement_force.x, swarm_forceMagnitude };
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

		// Ensure that we're not dealing with the player or the swarm enemy itself
		if ((!registry.players.has(entity)) && (!(entity == swarmEnemy))) {
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


