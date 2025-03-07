// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include "world_system.hpp"

// Constructor
PhysicsSystem::PhysicsSystem(b2WorldId worldId) : worldId(worldId)
{
}

// Destructor
PhysicsSystem::~PhysicsSystem() {}

// Camera Variables
float camera_next_step = 0.f;      // The next step in the camera's movement
float camera_objective_loc = -1.f; // Where the camera wants to end up (X-axis)
int shift_index = 1;               // What stage of the camera's movement it is in (X-axis)
bool speedy = false;               // Whether the player is moving faster than QUICK_MOVEMENT_THRESHOLD
float prev_x = 0.f;                // The previous x position of the camera
float prev_y = 0.f;                // The previous y position of the camera
float center_y = -1.f;             // The center y position of the camera
vec2 grapple_shift = { 0.f, 0.f }; // What stage of the camera's movement it is in towards grapple
float reset_shift = 0.f;           // What stage of the camera's movement it is in after grapple
bool after_grapple = false;        // Whether the camera is resetting after a grapple

// Camera Constants
float QUICK_MOVEMENT_THRESHOLD = 700.f;
float HORIZONTAL_FOCAL_SHIFT = 200.f;
float CAMERA_DELAY = 100.f;      // Higher = Slower camera movement
float VERTICAL_THRESHOLD = 50.f; // Lower = Camera will follow more aggressively

// M1 Linear Interpolation for Camera Movement
// See: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/a-brief-introduction-to-lerp-r4954/
float lerp(float start, float end, float t)
{
  return start * (1 - t) + end * t;
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
  // abs is to avoid negative scale due to the facing direction.
  return {abs(motion.scale.x), abs(motion.scale.y)};
}


// NOTE THAT THIS SHOULD REALLY ONLY BE CALLED WITH EITHER PLAYER OR ENEMY, AS IT DEPENDS ON ENTITIES HAVING ONE SHAPE!!!
// TODO NOTE: playtested & found an issue where the player and enemy occasionally phase through each other. Tried replicating by seeing if it triggers by jump, consecutive enemies, 
// etc, But it occurs seemingly randomly. Also encountered the issue where enemies occasionally phase through the ramp. These two issues could be related...?
// NOTE 2: Increased scale of both the player and enemy entities. Confirmed that if a collision happens off-center (say, the edges touch), it will not register, 
// but will if the centers of the 2 bodies are sufficiently close. This suggests that the on-screen geometry is not equal to the collision geometry, causing the phase-through 
// issue.
// NOTE 3: Tried decreasing scale of both player and enemy entities so the geometry on-screen is smaller than collision geometry. Confirmed that this remedied the phase-through 
// issue.
bool collides(const Entity &entity1, const Entity &entity2)
{
    // Get body IDs to identify entities in box2D
    b2BodyId entity1_id = registry.physicsBodies.get(entity1).bodyId;
    b2BodyId entity2_id = registry.physicsBodies.get(entity2).bodyId;
    
    // Figure out the shape IDs
    // Entity 1
    int entity1_numShapes = b2Body_GetShapeCount(entity1_id);
    b2ShapeId* entity1_shapeArray = new b2ShapeId[entity1_numShapes];
    b2Body_GetShapes(entity1_id, entity1_shapeArray, entity1_numShapes);
    b2ShapeId entity1_shape = entity1_shapeArray[0]; 
    // Entity 2
    int entity2_numShapes = b2Body_GetShapeCount(entity2_id);
    b2ShapeId* entity2_shapeArray = new b2ShapeId[entity2_numShapes];
    b2Body_GetShapes(entity2_id, entity2_shapeArray, entity2_numShapes);
    b2ShapeId entity2_shape = entity2_shapeArray[0];

    // Get contact data for both entities
    int entity1_numContacts = b2Body_GetContactCapacity(entity1_id);
    int entity2_numContacts = b2Body_GetContactCapacity(entity2_id);
    b2ContactData* entity1_contactData = new b2ContactData[entity1_numContacts];
    b2ContactData* entity2_contactData = new b2ContactData[entity2_numContacts];
    b2Body_GetContactData(entity1_id, entity1_contactData, entity1_numContacts);
    b2Body_GetContactData(entity2_id, entity2_contactData, entity2_numContacts);

    // Obvious case where if either of these entities have 0 contacts, then it must be the case that they're not colliding.
    if (entity1_numContacts == 0 || entity2_numContacts == 0) {
        return false;
    }

    // If they have contacts then it's less obvious. 
    // Iterate over every contact of either entity (as it should be reciprocal if they touch) and check if it contains the shapeID of the other. 
    // If they have the other's shapeID then they're definitely colliding.
    // We'll just check entity 1.
    for (int i = 0; i < entity1_numContacts; i++) {
        b2ContactData contact = entity1_contactData[i];

        // confirming both entities are in the collision
        if (((contact.shapeIdA.index1 == entity1_shape.index1 || contact.shapeIdB.index1 == entity1_shape.index1)) && // confirm that entity1 is one of the shapes involved
            ((contact.shapeIdA.index1 == entity2_shape.index1 || contact.shapeIdB.index1 == entity2_shape.index1)) // confirm that entity2 is one of the shapes involved
            ) 
        {
            return true;
        }
    }

    // if loop found nothing then no collision.
    return false;
}

// Advances physics simulation
void PhysicsSystem::step(float elapsed_ms)
{
  // To make things clearer, we'll separate player and enemy entities. Can refactor later to group them up.

  // Share this
  // Box2D v3 Upgrade: Use `b2World_Step()` instead of `world.Step()`
  float timeStep = elapsed_ms / 1000.0f;
  b2World_Step(worldId, timeStep, 4); // 4 is the recommended substep count
  // collisions and other events detected in b2World_Step()

  // Access physics body registry
  auto &physicsBody_registry = registry.physicsBodies;

  // PLAYER ENTITY
  // Access player registry
  auto &player_registry = registry.players;
  Player &player = player_registry.components[0];

  Entity playerEntity_physicsBody = player_registry.entities[0];

  PhysicsBody &playerComponent_physicsBody = registry.physicsBodies.get(playerEntity_physicsBody);
  b2BodyId playerBodyID = playerComponent_physicsBody.bodyId;
  b2Vec2 playerPosition = b2Body_GetPosition(playerBodyID);
  // Update motion component
  Motion &playerComponent_motion = registry.motions.get(playerEntity_physicsBody);
  playerComponent_motion.position = vec2(playerPosition.x, playerPosition.y);

  // ENEMY ENTITIES.
  //
  auto &enemy_registry = registry.enemies; // list of enemy entities stored in here

  // Iterate over every enemy entity to make them affected by Box2D physics.
  for (int i = 0; i < enemy_registry.entities.size(); i++)
  {

    Entity enemy_entity = enemy_registry.entities[i];

    // Get box2D stuff from enemy entity
    PhysicsBody &enemy_physicsBody = registry.physicsBodies.get(enemy_entity);
    b2BodyId enemyBodyID = enemy_physicsBody.bodyId;
    b2Vec2 enemyPosition = b2Body_GetPosition(enemyBodyID);

    // Update motion component of enemy entity
    Motion &enemyMotion = registry.motions.get(enemy_entity);
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
  Camera &camera = registry.cameras.get(playerEntity_physicsBody);

  float camX = playerPosition.x;
  float camY = playerPosition.y;

  // Push camera ahead when moving fast horizontally (Right)
  // std::cout << "Player velocity = (" << b2Body_GetLinearVelocity(bodyId).x << ", " << b2Body_GetLinearVelocity(bodyId).y << ")\n";
  if (b2Body_GetLinearVelocity(playerBodyID).x > QUICK_MOVEMENT_THRESHOLD)
  {
    speedy = true;
    // Initialize camera movement
    camera_next_step = lerp(playerPosition.x, playerPosition.x + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
    camera_objective_loc = playerPosition.x + HORIZONTAL_FOCAL_SHIFT;
    if (camera_next_step < camera_objective_loc)
    {
      camX = camera_next_step;
      shift_index++;
    }
    else
    {
      camX = camera_objective_loc;
    }
  }
  // Push camera ahead when moving fast horizontally (Left)
  else if (b2Body_GetLinearVelocity(playerBodyID).x < -QUICK_MOVEMENT_THRESHOLD)
  {
    speedy = true;
    // Initialize camera movement
    camera_next_step = lerp(playerPosition.x, playerPosition.x - HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
    camera_objective_loc = playerPosition.x - HORIZONTAL_FOCAL_SHIFT;
    if (camera_next_step > camera_objective_loc)
    {
      camX = camera_next_step;
      shift_index++;
    }
    else
    {
      camX = camera_objective_loc;
    }
  }
  else
  {
    speedy = false; // No longer going ham
  }

  // Slowly reset camera if no longer above movement threshold
  if (!speedy && playerPosition.x != camera_objective_loc)
  {
    if (playerPosition.x < camera_objective_loc)
    {
      camera_objective_loc = playerPosition.x + HORIZONTAL_FOCAL_SHIFT;
      camX = lerp(camX, camX + HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
    }
    else if (playerPosition.x > camera_objective_loc)
    {
      camera_objective_loc = playerPosition.x - HORIZONTAL_FOCAL_SHIFT;
      camX = lerp(camX, camX - HORIZONTAL_FOCAL_SHIFT / CAMERA_DELAY, shift_index);
    }
    if (shift_index > 1)
    {
      shift_index--;
    }
    else
    {
      camera_objective_loc = -1.f; // Reset objective location
    }
  }

  // Only move camera vertically if the player moves past a certain threshold
  // See: https://info.sonicretro.org/File:SPGCameraAir.gif
  // Moving Up
  if (playerPosition.y > prev_y)
  {
    // Save the center for reference
    if (center_y == -1.f)
    {
      center_y = prev_y;
    }

    // Do not move camera if player is not airborne yet
    if (playerPosition.y > center_y && playerPosition.y < center_y + VERTICAL_THRESHOLD)
    {
      camY = center_y;
    }
    // Move camera if player is sufficiently airborne
    else if (playerPosition.y >= center_y + VERTICAL_THRESHOLD)
    {
      center_y = -1.f; // Reset center
      camY = playerPosition.y - VERTICAL_THRESHOLD;
    }
  }
  // Moving Down
  else if (playerPosition.y < prev_y)
  {
    // Save the center for reference
    if (center_y == -1.f)
    {
      center_y = prev_y;
    }

    // Do not move camera if player has not fallen enough
    if (playerPosition.y < center_y && playerPosition.y > center_y - VERTICAL_THRESHOLD)
    {
      camY = center_y;
    }
    // Move camera if player has fallen far enough
    else if (playerPosition.y <= center_y - VERTICAL_THRESHOLD)
    {
      center_y = -1.f; // Reset center
      camY = playerPosition.y + VERTICAL_THRESHOLD;
    }
  }

  // Get grapple point position (static for now)
  Entity grapplePointEntity = registry.grapplePoints.entities[0];
  PhysicsBody& grappleBody = registry.physicsBodies.get(grapplePointEntity);
  b2BodyId grappleBodyId = grappleBody.bodyId;
  b2Vec2 grapplePos = b2Body_GetPosition(grappleBodyId);

  // Move camera towards grapple point
  if (grappleActive) {
      // Initialize variables
      camX = lerp(prev_x, grapplePos.x, grapple_shift.x);
	  camY = lerp(prev_y, grapplePos.y, grapple_shift.y);

	  if (camX != grapplePos.x || camY != grapplePos.y) {
          grapple_shift += 0.02;
	  }
  }
  // Reset camera back to player
  else {
      if (grapple_shift != vec2(0.f, 0.f)) {
          after_grapple = true;
          grapple_shift = { 0.f, 0.f };
      }
  }

  // Hard coded for now, will change to be dynamic later
  float LEFT_BOUNDARY = WINDOW_WIDTH_PX / 2.f;
  float RIGHT_BOUNDARY = WINDOW_WIDTH_PX * 2.5f;
  float TOP_BOUNDARY = WINDOW_HEIGHT_PX / 2.f;

  // Unlock the camera from the player if they approach the edge of world
  // This happens last because it has the highest priority
  if (camX < LEFT_BOUNDARY)
  {
      camX = WINDOW_WIDTH_PX / 2.f;
  }
  if (camX > RIGHT_BOUNDARY)
  {
      camX = WINDOW_WIDTH_PX * 2.5f;
  }
  if (camY > TOP_BOUNDARY)
  {
      camY = WINDOW_HEIGHT_PX / 2.f;
  }

  // If the camera is post-grapple, we need to dynamically move it back 
  // to wherever it needs to go
  if (after_grapple)
  {
      // If we have reached destination or player re-grapples, stop the process
      if (reset_shift >= 1.0f || grappleActive) {
          after_grapple = false;
		  reset_shift = 0.f;
      }
      else {
          camX = lerp(prev_x, camX, reset_shift);
          camY = lerp(prev_y, camY, reset_shift);
          reset_shift += 0.001;
      }
  }


  camera.position = vec2(camX, camY);

  prev_x = camX;
  prev_y = camY;

  // Debugging output
  // std::cout << "Box2D Ball Body position = (" << position.x << ", " << position.y << ")\n";

  // COLLISION HANDLING
  // This just iterates over all motion entities to check. 
  // The collision check is handled by collides() helper function.
  ComponentContainer<Motion> &motion_container = registry.motions;
  for (uint i = 0; i < motion_container.components.size(); i++)
  {
    Motion &motion_i = motion_container.components[i];
    Entity entity_i = motion_container.entities[i];

    // note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
    for (uint j = i + 1; j < motion_container.components.size(); j++)
    {
      Motion &motion_j = motion_container.components[j];
      Entity entity_j = motion_container.entities[j];
      // We really only want to check collisions if both of these are either players or enemies.
      if ((registry.players.has(entity_i) || registry.enemies.has(entity_i)) && // entity i is either a player or an enemy
          (registry.players.has(entity_j) || registry.enemies.has(entity_j)) // entity j is either a player or an enemy
          ) {
          if (collides(entity_i, entity_j))
          {
              // Now that we know the 2 entities are colliding we also want to figure out which entity comes out on top.
              // Current criteria for "top" is higher speed during collision.
              
              // Get body IDs to identify entities in box2D (note that i, j, maps onto 1, 2)
              b2BodyId entity1_id = registry.physicsBodies.get(entity_i).bodyId;
              b2BodyId entity2_id = registry.physicsBodies.get(entity_j).bodyId;
              // Get speeds
              b2Vec2 entity1_velocity = b2Body_GetLinearVelocity(entity1_id);
              float entity1_speedFactor = (entity1_velocity.x * entity1_velocity.x) + (entity1_velocity.y * entity1_velocity.y);
              b2Vec2 entity2_velocity = b2Body_GetLinearVelocity(entity2_id);
              float entity2_speedFactor = (entity2_velocity.x * entity2_velocity.x) + (entity2_velocity.y * entity2_velocity.y);

              // Figure out the winning entity in this collision
              // For now it's just the one with higher speed. Eventually we might also want to consider kinetic energy instead to make larger enemies harder to kill.
              Entity winner; 
              if (entity1_speedFactor > entity2_speedFactor) {
                  winner = entity_i;
              }
              else {
                  winner = entity_j;
              }

              // Create a collisions event
              // We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
              // CK: why the duplication, except to allow searching by entity_id
              Collision& collision = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
              collision.winner = winner;
          }
      }
      
    }
  }

}
