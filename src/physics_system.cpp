// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include "world_system.hpp"
#include <glm/trigonometric.hpp>

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
vec2 grapple_shift = {0.f, 0.f};   // What stage of the camera's movement it is in towards grapple
float reset_shift = 0.f;           // What stage of the camera's movement it is in after grapple
bool after_grapple = false;        // Whether the camera is resetting after a grapple
int camera_panned = 0;             // 0 = Centered, 1 = Panned Right, 2 = Panned Left

// Camera Constants
float QUICK_MOVEMENT_THRESHOLD = 900.f;
float HORIZONTAL_FOCAL_SHIFT = 200.f;
float CAMERA_SPEED = 5.f;        // Lower = Slower camera movement
float VERTICAL_THRESHOLD = 50.f; // Lower = Camera will follow more aggressively
float DEFAULT = -1.f;

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
  b2ShapeId *entity1_shapeArray = new b2ShapeId[entity1_numShapes];
  b2Body_GetShapes(entity1_id, entity1_shapeArray, entity1_numShapes);
  b2ShapeId entity1_shape = entity1_shapeArray[0];
  // Entity 2
  int entity2_numShapes = b2Body_GetShapeCount(entity2_id);
  b2ShapeId *entity2_shapeArray = new b2ShapeId[entity2_numShapes];
  b2Body_GetShapes(entity2_id, entity2_shapeArray, entity2_numShapes);
  b2ShapeId entity2_shape = entity2_shapeArray[0];

  // Get contact data for both entities
  int entity1_numContacts = b2Body_GetContactCapacity(entity1_id);
  int entity2_numContacts = b2Body_GetContactCapacity(entity2_id);
  b2ContactData *entity1_contactData = new b2ContactData[entity1_numContacts];
  b2ContactData *entity2_contactData = new b2ContactData[entity2_numContacts];
  b2Body_GetContactData(entity1_id, entity1_contactData, entity1_numContacts);
  b2Body_GetContactData(entity2_id, entity2_contactData, entity2_numContacts);

  // Obvious case where if either of these entities have 0 contacts, then it must be the case that they're not colliding.
  if (entity1_numContacts == 0 || entity2_numContacts == 0)
  {
    return false;
  }

  // If they have contacts then it's less obvious.
  // Iterate over every contact of either entity (as it should be reciprocal if they touch) and check if it contains the shapeID of the other.
  // If they have the other's shapeID then they're definitely colliding.
  // We'll just check entity 1.
  for (int i = 0; i < entity1_numContacts; i++)
  {
    b2ContactData contact = entity1_contactData[i];

    // confirming both entities are in the collision
    if (((contact.shapeIdA.index1 == entity1_shape.index1 || contact.shapeIdB.index1 == entity1_shape.index1)) && // confirm that entity1 is one of the shapes involved
        ((contact.shapeIdA.index1 == entity2_shape.index1 || contact.shapeIdB.index1 == entity2_shape.index1))    // confirm that entity2 is one of the shapes involved
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
  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  // Freeze physics if we're not playing
  if (currentScreen.current_screen != "PLAYING")
  {
    return;
  }

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

  // Update player position
  b2Vec2 playerPosition = b2Body_GetPosition(playerBodyID);
  Motion &playerComponent_motion = registry.motions.get(playerEntity_physicsBody);
  playerComponent_motion.position = vec2(playerPosition.x, playerPosition.y);

  // Update player rotation
  b2Rot rotation = b2Body_GetRotation(playerBodyID);
  float angleRadians = b2Rot_GetAngle(rotation);
  playerComponent_motion.angle = glm::degrees(angleRadians); 

  // Update rotatable sprite layers related to the player
  for (int i = 0; i < registry.playerRotatableLayers.entities.size(); i++)
  {
      Entity rotatableLayer = registry.playerRotatableLayers.entities[i];
      Motion& rotatableMotion = registry.motions.get(rotatableLayer);
      rotatableMotion.position = vec2(playerPosition.x, playerPosition.y);
      rotatableMotion.angle = playerComponent_motion.angle;
  }

  // Special behavior for the Ramster sprite layers
  for (int i = 0; i < registry.playerNonRotatableLayers.entities.size(); i++)
  {
      Entity nonRotatableLayer = registry.playerNonRotatableLayers.entities[i];
      Motion& nonRotatableMotion = registry.motions.get(nonRotatableLayer);
      nonRotatableMotion.position = vec2(playerPosition.x, playerPosition.y);

      if (registry.runAnimations.has(nonRotatableLayer)) {
          // Tilt angle based on velocity
          b2Vec2 velocity = b2Body_GetLinearVelocity(playerBodyID);
          float maxTiltAngle = 15.f;
          float tilt = -(glm::clamp(velocity.x * 3.f, -maxTiltAngle, maxTiltAngle));
          nonRotatableMotion.angle = glm::mix(nonRotatableMotion.angle, tilt, 0.25f);

          // Set animation frame time based on speed
          RenderRequest& rr = registry.renderRequests.get(nonRotatableLayer);
          float speed = b2Length(velocity);

          const float minFrameTime = 50.f;   // Faster animation when moving quickly
          const float maxFrameTime = 300.f;  // Slower animation when stationary

          float calculatedFrameTime = maxFrameTime - speed * 0.25;

          rr.animation_frame_time = glm::clamp(calculatedFrameTime, minFrameTime, maxFrameTime);
      }
  }

  // ENEMY ENTITIES
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
  if (b2Body_GetLinearVelocity(playerBodyID).x > QUICK_MOVEMENT_THRESHOLD && camera_panned != 2)
  {
    speedy = true;
    camera_panned = 1;
    // Initialize camera movement
    camera_next_step = playerPosition.x + CAMERA_SPEED * shift_index;
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
  else if (b2Body_GetLinearVelocity(playerBodyID).x < -QUICK_MOVEMENT_THRESHOLD && camera_panned != 1)
  {
    speedy = true;
    camera_panned = 2;
    // Initialize camera movement
    camera_next_step = playerPosition.x - CAMERA_SPEED * shift_index;
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
  if (!speedy && camera_objective_loc != DEFAULT)
  {
    if (playerPosition.x < camera_objective_loc)
    {
      camera_objective_loc = playerPosition.x + HORIZONTAL_FOCAL_SHIFT;
      camX = playerPosition.x + CAMERA_SPEED * shift_index;
    }
    else if (playerPosition.x > camera_objective_loc)
    {
      camera_objective_loc = playerPosition.x - HORIZONTAL_FOCAL_SHIFT;
      camX = playerPosition.x - CAMERA_SPEED * shift_index;
    }
    if (shift_index > 1)
    {
      shift_index--;
    }
    else
    {
      camera_objective_loc = DEFAULT; // Reset objective location
      camera_panned = 0;              // Reset panning
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
  // Move camera towards grapple point
  if (grapplePointActive)
  {
    // Initialize variables
    Entity activeGrapplePointEntity;
    b2BodyId activeGrappleBodyId;

    // Loop through all grapple enemies_killed and find the active one
    for (Entity gpEntity : registry.grapplePoints.entities)
    {
      GrapplePoint &gp = registry.grapplePoints.get(gpEntity);
      // std::cout << gp.position.x << " " << gp.position.y <<  " " << gp.active << std::endl;
      if (gp.active)
      {
        activeGrapplePointEntity = gpEntity;
        activeGrappleBodyId = gp.bodyId;
      }
    }

    b2Vec2 grapplePos = b2Body_GetPosition(activeGrappleBodyId);
    camX = lerp(prev_x, grapplePos.x, grapple_shift.x);
    camY = lerp(prev_y, grapplePos.y, grapple_shift.y);

    if (camX != grapplePos.x || camY != grapplePos.y)
    {
        grapple_shift += 0.02;
    }
    
  }
  // Reset camera back to player
  else
  {
    if (grapple_shift != vec2(0.f, 0.f))
    {
      after_grapple = true;
      grapple_shift = {0.f, 0.f};
    }
  }

  // Hard coded for now, will change to be dynamic later
  float LEFT_BOUNDARY = WINDOW_WIDTH_PX / 2.f;
  float RIGHT_BOUNDARY = WORLD_WIDTH_PX - (WINDOW_WIDTH_PX / 2.f);
  float TOP_BOUNDARY = WORLD_HEIGHT_PX - (WINDOW_HEIGHT_PX / 2.f);

  // Unlock the camera from the player if they approach the edge of world
  // This happens last because it has the highest priority
  if (camX < LEFT_BOUNDARY && !grapplePointActive)
  {
    camX = LEFT_BOUNDARY;
  }
  if (camX > RIGHT_BOUNDARY && !grapplePointActive)
  {
    camX = RIGHT_BOUNDARY;
  }
  if (camY > TOP_BOUNDARY && !grapplePointActive)
  {
    camY = TOP_BOUNDARY;
  }

  // If the camera is post-grapple, we need to dynamically move it back
  // to wherever it needs to go
  if (after_grapple)
  {
    // If we have reached destination or player re-grapples, stop the process
    if (reset_shift >= 1.0f || grapplePointActive)
    {
      after_grapple = false;
      reset_shift = 0.f;
    }
    else
    {
      camX = lerp(prev_x, camX, reset_shift);
      camY = lerp(prev_y, camY, reset_shift);
      reset_shift += 0.02;
    }
  }

  // also update the parallax background to be in sync with the player
  auto &background_registry = registry.backgroundLayers;
  BackgroundLayer &backgroundLayer = background_registry.components.back();
  Entity background_entity = background_registry.entities.back();
  Motion &background_motion = registry.motions.get(background_entity);
  background_motion.position = vec2(camX, camY);

  camera.position = vec2(camX, camY);

  prev_x = camX;
  prev_y = camY;

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
              float entity1_speedFactor = ((entity1_velocity.x * entity1_velocity.x) + (entity1_velocity.y * entity1_velocity.y))/100000;
              b2Vec2 entity2_velocity = b2Body_GetLinearVelocity(entity2_id);
              float entity2_speedFactor = ((entity2_velocity.x * entity2_velocity.x) + (entity2_velocity.y * entity2_velocity.y))/100000;

              // ID the player
              Entity playerEntity;
              b2Vec2 playerVelocity;
              if (registry.players.has(entity_i)) {
                playerVelocity = entity1_velocity;
                playerEntity = entity_i;
              }
              else {
                playerVelocity = entity2_velocity;
                playerEntity = entity_j;
              }

              // To determine if the player "won" in that collision, what we ultimately want to find out is whether the player was moving fast enough.
              // So, at a high enough speed, the player should be relatively resistant to damage. The goal of the enemy would then be to absorb as much of
              // the player's speed as possible. 
              // Since the player is bouncy and our enemies are not very fast (at least not fast enough to make the player surpass the required speed to "win"),
              // we can figure out if the player is fast enough by just checking on how much speed they retain after the collision.
              // NOTE: this depends on MIN_COLLISION_SPEED, which will need some fine-tuning to find a good speed at which we can hit the enemy.
              bool player_wins_collision = false;

              if (b2Length(playerVelocity) > MIN_COLLISION_SPEED * 0.9) {
                  player_wins_collision = true;
              }

              // Create a collisions event
              // We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
              // CK: why the duplication, except to allow searching by entity_id
              Collision& collision = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
              collision.player_wins_collision = player_wins_collision;
          }
      }
    }
  }

  if (grappleActive)
  {
    updateGrappleLines();
  }

  update_player_animation();
  update_fireball();
  updateHealthBar(camera.position);
  updateTimer(camera.position);
  updateScore(camera.position);
}

void PhysicsSystem::updateGrappleLines()
{
  for (Entity grappleEntity : registry.grapples.entities)
  {
    Grapple &grapple = registry.grapples.get(grappleEntity);

    // Get current positions
    b2Vec2 ballPos = b2Body_GetPosition(grapple.ballBodyId);
    b2Vec2 grapplePos = b2Body_GetPosition(grapple.grappleBodyId);

    // Update line entity positions
    if (registry.lines.has(grapple.lineEntity))
    {
      Line &line = registry.lines.get(grapple.lineEntity);
      line.start_pos = vec2(ballPos.x, ballPos.y);
      line.end_pos = vec2(grapplePos.x, grapplePos.y);
    }
  }
}

void PhysicsSystem::update_fireball()
{
  // Check if there is a player entity
  if (registry.players.entities.empty() || registry.fireballs.entities.empty())
  {
    return;
  }

  // Get the player entity and its motion and physics components
  Entity playerEntity = registry.players.entities[0];
  Motion &playerMotion = registry.motions.get(playerEntity);
  PhysicsBody &playerPhysics = registry.physicsBodies.get(playerEntity);

  // Get the player's velocity from Box2D
  b2Vec2 playerVelocity = b2Body_GetLinearVelocity(playerPhysics.bodyId);
  float playerSpeed = b2Length(playerVelocity);

  b2Vec2 playerDirection = b2Normalize(playerVelocity);

  const float fireballAspectRatio = 774.f / 260.f;

  // Check if the player is moving at or above the minimum collision speed
  if (playerSpeed >= MIN_COLLISION_SPEED)
  {
    // Set the fireball render request to visible
    for (Entity fireballEntity : registry.fireballs.entities)
    {
      RenderRequest &fireballRenderRequest = registry.renderRequests.get(fireballEntity);
      fireballRenderRequest.is_visible = true;

      // Adjust the fireball's position to be slightly behind the ball's current position
      Motion &fireballMotion = registry.motions.get(fireballEntity);
      vec2 offset = vec2(-playerDirection.x, -playerDirection.y) * 60.f;
      fireballMotion.position = playerMotion.position + offset;

      // Rotate the fireball to point in the same direction as the ball's movement
      float angle = atan2(playerDirection.y, playerDirection.x) * (180.f / M_PI);
      fireballMotion.angle = angle;
    }
  }
  else
  {
    // If the ball is not moving or below the threshold, set the fireball's position to the same as the ball
    for (Entity fireballEntity : registry.fireballs.entities)
    {
      Motion &fireballMotion = registry.motions.get(fireballEntity);
      fireballMotion.position = playerMotion.position;

      // Set the fireball render request to not visible
      RenderRequest &fireballRenderRequest = registry.renderRequests.get(fireballEntity);
      fireballRenderRequest.is_visible = false;
    }
  }
}

void PhysicsSystem::updateHealthBar(vec2 camPos)
{
  for (Entity hpEntity : registry.healthbars.entities)
  {
    HealthBar &hp = registry.healthbars.get(hpEntity);

    Motion &motion = registry.motions.get(hpEntity);
    float hp_ratio = std::max(0.f, hp.health / 5.f);
    float full_width = 200.f;
    float bar_width = full_width * hp_ratio;

    // shrink leftward: adjust position to keep left side fixed
    float offset = (full_width - bar_width) / 2.f;

    motion.scale.x = bar_width;
    motion.position = vec2(camPos.x - WINDOW_WIDTH_PX / 2 + 150.0f - offset,
                           camPos.y + WINDOW_HEIGHT_PX / 2 - 40.0f);
  }
}

void PhysicsSystem::updateScore(vec2 camPos)
{
  const float digitSpacing = 4.f;
  const float digitWidth = 30.f;
  const float fullDigitWidth = digitWidth + digitSpacing;
  const float rightMargin = 40.f;

  const float rightEdge = camPos.x + WINDOW_WIDTH_PX / 2.f - rightMargin;
  const float baseY = camPos.y + WINDOW_HEIGHT_PX / 2.f - 40.f - 60.0f;

  for (Entity scoreEntity : registry.scores.entities)
  {
    Score &score = registry.scores.get(scoreEntity);

    for (int i = 0; i < 4; ++i)
    {
      Entity digitEntity = score.digits[i];
      Motion &motion = registry.motions.get(digitEntity);

      float x = rightEdge - (3 - i) * fullDigitWidth;
      motion.position = vec2(x, baseY);
    }
  }
}

void PhysicsSystem::updateTimer(vec2 camPos)
{
  const float digitSpacing = 4.f;
  const float digitWidth = 30.f;
  const float fullDigitWidth = digitWidth + digitSpacing;
  const float rightMargin = 40.f;

  const float rightEdge = camPos.x + WINDOW_WIDTH_PX / 2.f - rightMargin;
  const float baseY = camPos.y + WINDOW_HEIGHT_PX / 2.f - 40.f;

  for (Entity timerEntity : registry.timers.entities)
  {
    Timer &timer = registry.timers.get(timerEntity);

    for (int i = 0; i < 7; ++i)
    {
      Entity digitEntity = timer.digits[i];
      Motion &motion = registry.motions.get(digitEntity);

      float x = rightEdge - (6 - i) * fullDigitWidth;
      motion.position = vec2(x, baseY);
    }
  }
}

void PhysicsSystem::update_player_animation() {
    // Check if there is a player entity
    if (registry.players.entities.empty() ||
        registry.idleAnimations.entities.empty() ||
        registry.runAnimations.entities.empty()) {
        return;
    }

    // Get the player entity and its motion and physics components
    Entity playerEntity = registry.players.entities[0];
    Motion& playerMotion = registry.motions.get(playerEntity);
    PhysicsBody& playerPhysics = registry.physicsBodies.get(playerEntity);

    // Get the player's velocity from Box2D
    b2Vec2 playerVelocity = b2Body_GetLinearVelocity(playerPhysics.bodyId);
    float playerSpeed = b2Length(playerVelocity);

    // Switch between running and idle animation based on player speed
    if (playerSpeed > 20.0) {
        for (Entity runAnimation : registry.runAnimations.entities) {
            RenderRequest& runRenderRequest = registry.renderRequests.get(runAnimation);
            runRenderRequest.is_visible = true;
        }
        for (Entity idleAnimation : registry.idleAnimations.entities) {
            RenderRequest& idleRenderRequest = registry.renderRequests.get(idleAnimation);
            idleRenderRequest.is_visible = false;
        }
    }
    else {
        for (Entity idleAnimation : registry.idleAnimations.entities) {
            RenderRequest& idleRenderRequest = registry.renderRequests.get(idleAnimation);
            idleRenderRequest.is_visible = true;
        }
        for (Entity runAnimation : registry.runAnimations.entities) {
            RenderRequest& runRenderRequest = registry.renderRequests.get(runAnimation);
            runRenderRequest.is_visible = false;
        }
    }
}

