#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

#include <box2d/box2d.h>

// enemy
Entity createEnemy(b2WorldId worldID, vec2 pos);

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

// grapple hook
Entity createGrapple(b2WorldId worldId, b2BodyId ballBodyId, b2BodyId grappleBodyId, float distance);
void removeGrapple();

// legacy
// the player
Entity createChicken(RenderSystem* renderer, vec2 position);
Entity createBall(b2WorldId worldId);