#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

#include <box2d/box2d.h>


// current screen
Entity createCurrentScreen(); // DEFAULT TO MAIN MENU

// screen
Entity createScreen(std::string screen_type);

// enemy
Entity createEnemy(b2WorldId worldID, vec2 pos, ENEMY_TYPES enemy_type, vec2 movement_range_point_a, vec2 movement_range_point_b);

// invaders
Entity createInvader(RenderSystem* renderer, vec2 position);

// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// projectile
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity);

// explosion
Entity createExplosion(RenderSystem* renderer, vec2 position);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// terrain lines
Entity createLine(vec2 start_pos, vec2 end_pos);

// grapple point (andrew version)
Entity createGrapplePoint(b2WorldId worldId, vec2 position);

// grapple hook
Entity createGrapple(b2WorldId worldId, b2BodyId ballBodyId, b2BodyId grappleBodyId, float distance);
void removeGrapple();

Entity createGoalZone(vec2 bottom_left_pos, vec2 bottom_right_pos);

// level layers
Entity createLevelTextureLayer(TEXTURE_ASSET_ID textureId);
Entity createBackgroundLayer(TEXTURE_ASSET_ID textureId);

// legacy
// the player
Entity createChicken(RenderSystem* renderer, vec2 position);
Entity createBall(b2WorldId worldId, vec2 startPos);