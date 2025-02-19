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

    // === UPDATE CAMERA POSITION ===
    // The camera has the following unique features:
    //  - Default follows the player in the center
	//  - When moving fast horizontally, the camera pushes ahead to show more of the level
	//  - When no longer moving fast, the camera gradually resets to the center
    //  - Vertically the camera will only start following after jumping or falling a certain distance
	//  - When reaching the edge of the world, the camera locks on the boundary of the level (except for ground)
    // @Zach
    Camera& camera = registry.cameras.get(entity_physicsBody);

	float camX = position.x;
	float camY = position.y;

    // Push camera ahead when moving fast horizontally (Right)
	std::cout << "Player velocity = (" << b2Body_GetLinearVelocity(bodyId).x << ", " << b2Body_GetLinearVelocity(bodyId).y << ")\n";
    if (b2Body_GetLinearVelocity(bodyId).x > QUICK_MOVEMENT_THRESHOLD) {
        speedy = true;
        // Initialize camera movement
        camera_next_step = lerp(position.x, position.x + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
        camera_objective_loc = position.x + HORIZONTAL_FOCAL_SHIFT;
		if (camera_next_step < camera_objective_loc) {
			camX = camera_next_step;
			shift_index++;
        }
        else {
			camX = camera_objective_loc;
        }
	}
	// Push camera ahead when moving fast horizontally (Left)
	else if (b2Body_GetLinearVelocity(bodyId).x < -QUICK_MOVEMENT_THRESHOLD) {
        speedy = true;
        // Initialize camera movement
		camera_next_step = lerp(position.x, position.x - HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
        camera_objective_loc = position.x - HORIZONTAL_FOCAL_SHIFT;
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
	if (!speedy && position.x != camera_objective_loc) {
		if (position.x < camera_objective_loc) {
            camera_objective_loc = position.x + HORIZONTAL_FOCAL_SHIFT;
			camX = lerp(camX, camX + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
		}
		else if (position.x > camera_objective_loc) {
            camera_objective_loc = position.x - HORIZONTAL_FOCAL_SHIFT;
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
	if (position.y > prev_y) {
        // Save the center for reference
        if (center_y == -1.f) {
			center_y = prev_y;
        }

        // Do not move camera if player is not airborne yet
        if (position.y > center_y && position.y < center_y + VERTICAL_THRESHOLD) {
            camY = center_y;
		}
		// Move camera if player is sufficiently airborne
		else if (position.y >= center_y + VERTICAL_THRESHOLD) {
			center_y = -1.f; // Reset center
			camY = position.y - VERTICAL_THRESHOLD;
		}
	}
    // Moving Down
    else if (position.y < prev_y) {
        // Save the center for reference
        if (center_y == -1.f) {
            center_y = prev_y;
        }

        // Do not move camera if player has not fallen enough
        if (position.y < center_y && position.y > center_y - VERTICAL_THRESHOLD) {
            camY = center_y;
        }
        // Move camera if player has fallen far enough
        else if (position.y <= center_y - VERTICAL_THRESHOLD) {
            center_y = -1.f; // Reset center
            camY = position.y + VERTICAL_THRESHOLD;
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
}
