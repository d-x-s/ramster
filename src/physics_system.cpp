// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Constructor
PhysicsSystem::PhysicsSystem(b2WorldId worldId) : worldId(worldId) {

}

// Destructor
PhysicsSystem::~PhysicsSystem() {}

// Camera Variables
float camera_next_step = 0.f; // The next step in the camera's movement
float camera_objective_loc = -1.f; // Where the camera wants to end up (X-axis)
int shift_index = 1; // What stage of the camera's movement it is in (X-axis)
bool speedy = false; // Whether the player is moving faster than QUICK_MOVEMENT_THRESHOLD
float prev_y = 0.f; // The previous y position of the camera
float center_y = -1.f; // The center y position of the camera

// Camera Constants
float QUICK_MOVEMENT_THRESHOLD = 700.f;
float HORIZONTAL_FOCAL_SHIFT = 200.f;
float CAMERA_DELAY = 100.f; // Higher = Slower camera movement
float VERTICAL_THRESHOLD = 50.f; // Lower = Camera will follow more aggressively

// M1 Linear Interpolation for Camera Movement
float lerp(float start, float end, float t) {
	return start * (1 - t) + end * t;
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
    // abs is to avoid negative scale due to the facing direction.
    return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
    vec2 dp = motion1.position - motion2.position;
    float dist_squared = dot(dp, dp);
    const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
    const float other_r_squared = dot(other_bonding_box, other_bonding_box);
    const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
    const float my_r_squared = dot(my_bonding_box, my_bonding_box);
    const float r_squared = max(other_r_squared, my_r_squared);
    if (dist_squared < r_squared)
        return true;
    return false;
}

// Advances physics simulation
void PhysicsSystem::step(float elapsed_ms)
{
    // To make things clearer, we'll separate player and enemy entities. Can refactor later to group them up.
    
    // Share this
    // Box2D v3 Upgrade: Use `b2World_Step()` instead of `world.Step()`
    float timeStep = elapsed_ms / 1000.0f;
    b2World_Step(worldId, timeStep, 4);  // 4 is the recommended substep count
    // collisions and other events detected in b2World_Step()

    // Access physics body registry
    auto& physicsBody_registry = registry.physicsBodies;

    // PLAYER ENTITY
    // Access player registry
    auto& player_registry = registry.players;
    Player& player = player_registry.components[0];

    Entity playerEntity_physicsBody = player_registry.entities[0];

    PhysicsBody& playerComponent_physicsBody = registry.physicsBodies.get(playerEntity_physicsBody);
    b2BodyId playerBodyID = playerComponent_physicsBody.bodyId;
    b2Vec2 playerPosition = b2Body_GetPosition(playerBodyID);

    // Update motion component
    Motion& playerComponent_motion = registry.motions.get(playerEntity_physicsBody);
    playerComponent_motion.position = vec2(playerPosition.x, playerPosition.y);


    // ENEMY ENTITIES.
    //
    auto& enemy_registry = registry.enemies; //list of enemy entities stored in here

    // Iterate over every enemy entity to make them affected by Box2D physics.
    for (int i = 0; i < enemy_registry.entities.size(); i++) {

        Entity enemy_entity = enemy_registry.entities[i];

        // Get box2D stuff from enemy entity
        PhysicsBody& enemy_physicsBody = registry.physicsBodies.get(enemy_entity);
        b2BodyId enemyBodyID = enemy_physicsBody.bodyId;
        b2Vec2 enemyPosition = b2Body_GetPosition(enemyBodyID);

        // Update motion component of enemy entity
        Motion& enemyMotion = registry.motions.get(enemy_entity);
        enemyMotion.position = vec2(enemyPosition.x, enemyPosition.y);

    }


    // === UPDATE CAMERA POSITION ===
    // The camera has the following unique features:
    //  - Default follows the player in the center
	//  - When moving fast horizontally, the camera pushes ahead to show more of the level
	//  - When no longer moving fast, the camera gradually resets to the center
    //  - Vertically the camera will only start following after jumping or falling a certain distance
	//  - When reaching the edge of the world, the camera locks on the boundary of the level (except for ground)
    // @Zach
    Camera& camera = registry.cameras.get(playerEntity_physicsBody);

	float camX = playerPosition.x;
	float camY = playerPosition.y;

    // Push camera ahead when moving fast horizontally (Right)
	std::cout << "Player velocity = (" << b2Body_GetLinearVelocity(playerBodyID).x << ", " << b2Body_GetLinearVelocity(playerBodyID).y << ")\n";
    if (b2Body_GetLinearVelocity(playerBodyID).x > QUICK_MOVEMENT_THRESHOLD) {
        speedy = true;
        // Initialize camera movement
        camera_next_step = lerp(playerPosition.x, playerPosition.x + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
        camera_objective_loc = playerPosition.x + HORIZONTAL_FOCAL_SHIFT;
		if (camera_next_step < camera_objective_loc) {
			camX = camera_next_step;
			shift_index++;
        }
        else {
			camX = camera_objective_loc;
        }
	}
	// Push camera ahead when moving fast horizontally (Left)
	else if (b2Body_GetLinearVelocity(playerBodyID).x < -QUICK_MOVEMENT_THRESHOLD) {
        speedy = true;
        // Initialize camera movement
		camera_next_step = lerp(playerPosition.x, playerPosition.x - HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
        camera_objective_loc = playerPosition.x - HORIZONTAL_FOCAL_SHIFT;
        if (camera_next_step > camera_objective_loc) {
            camX = camera_next_step;
            shift_index++;
        }
        else {
            camX = camera_objective_loc;
        }
	}
	else {
		speedy = false; // No longer going ham
	}

    // Slowly reset camera if no longer above movement threshold
	if (!speedy && playerPosition.x != camera_objective_loc) {
		if (playerPosition.x < camera_objective_loc) {
            camera_objective_loc = playerPosition.x + HORIZONTAL_FOCAL_SHIFT;
			camX = lerp(camX, camX + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
		}
		else if (playerPosition.x > camera_objective_loc) {
            camera_objective_loc = playerPosition.x - HORIZONTAL_FOCAL_SHIFT;
			camX = lerp(camX, camX - HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
		}
        if (shift_index > 1) {
			shift_index--;
        }
        else {
			camera_objective_loc = -1.f; // Reset objective location
        }
	}

    // Only move camera vertically if the player moves past a certain threshold
    // See: https://info.sonicretro.org/File:SPGCameraAir.gif
    // Moving Up
	if (playerPosition.y > prev_y) {
        // Save the center for reference
        if (center_y == -1.f) {
			center_y = prev_y;
        }

        // Do not move camera if player is not airborne yet
        if (playerPosition.y > center_y && playerPosition.y < center_y + VERTICAL_THRESHOLD) {
            camY = center_y;
		}
		// Move camera if player is sufficiently airborne
		else if (playerPosition.y >= center_y + VERTICAL_THRESHOLD) {
			center_y = -1.f; // Reset center
			camY = playerPosition.y - VERTICAL_THRESHOLD;
		}
	}
    // Moving Down
    else if (playerPosition.y < prev_y) {
        // Save the center for reference
        if (center_y == -1.f) {
            center_y = prev_y;
        }

        // Do not move camera if player has not fallen enough
        if (playerPosition.y < center_y && playerPosition.y > center_y - VERTICAL_THRESHOLD) {
            camY = center_y;
        }
        // Move camera if player has fallen far enough
        else if (playerPosition.y <= center_y - VERTICAL_THRESHOLD) {
            center_y = -1.f; // Reset center
            camY = playerPosition.y + VERTICAL_THRESHOLD;
        }
    }

    // Hard coded for now, will change to be dynamic later
    float LEFT_BOUNDARY = WINDOW_WIDTH_PX / 2.f;
	float RIGHT_BOUNDARY = WINDOW_WIDTH_PX * 2.5f;
	float TOP_BOUNDARY = WINDOW_HEIGHT_PX / 2.f;

    // Unlock the camera from the player if they approach the edge of world
	// This happens last because it has the highest priority
	if (camX < LEFT_BOUNDARY) {
		camX = WINDOW_WIDTH_PX / 2.f;
	}
	if (camX > RIGHT_BOUNDARY) {
		camX = WINDOW_WIDTH_PX * 2.5f;
	} 
    if (camY > TOP_BOUNDARY) { 
		camY = WINDOW_HEIGHT_PX / 2.f;
    } 

    camera.position = vec2(camX, camY);

	prev_y = camY;

    // Debugging output
    // std::cout << "Box2D Ball Body position = (" << position.x << ", " << position.y << ")\n";


    // COLLISION HANDLING
    // Ran out of time, will use A1 for now, figure out collision callbacks later.
    // check for collisions between all moving entities
    ComponentContainer<Motion>& motion_container = registry.motions;
    for (uint i = 0; i < motion_container.components.size(); i++)
    {
        Motion& motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];

        // note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
        for (uint j = i + 1; j < motion_container.components.size(); j++)
        {
            Motion& motion_j = motion_container.components[j];
            if (collides(motion_i, motion_j))
            {
                Entity entity_j = motion_container.entities[j];
                // Create a collisions event
                // We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
                // CK: why the duplication, except to allow searching by entity_id
                registry.collisions.emplace_with_duplicates(entity_i, entity_j);
            }
        }
    }
    //WIP BOX2D CODE:
    /*
    * b2ContactEvents contactEvents = b2World_GetContactEvents(worldId);

    for (int i = 0; i < contactEvents.beginCount; i++) {
        b2ContactBeginTouchEvent event = contactEvents.beginEvents[i];
        b2BodyId collisionBody_A = b2Shape_GetBody(event.shapeIdA);
        b2BodyId collisionBody_B = b2Shape_GetBody(event.shapeIdB);

        std::cout << "TEST XXXXXXXXXXXXXXXXXXXXXXXX" << b2Body_GetUserData(collisionBody_A) << std::endl;
    }
    */
}
