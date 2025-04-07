#pragma once

#include "common.hpp"
#include <chrono>
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

#include <box2d/box2d.h>


// current screen
Entity createCurrentScreen(); // DEFAULT TO MAIN MENU

/* Creates an element to dispay on-screen.
	Takes:
	- screen: screen that this element is for
	- texture: texture of this element
	- width_px: width of this element
	- height_px: height of this element
	- pos_relative_center: position of element relative to CENTER of screen.
	
	NOTE: for width, height, and pos, make sure to enter values relative to viewport so scaling works!!!
	TIP: to get viewport-relative values, do this: (Viewport Pixels / Actual Pixels) Amount to Divide
*/
Entity createScreenElement(std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center);

/* Creates a button to display on-screen.
	Function is an identifier for the button so it can be recognized. Make sure to use one button per function.
	For remaining params, same logic/inputs as createScreenElement, except this makes a button.
*/
Entity createButton(std::string function, std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center);

// Makes buttons that are used to select levels. Specify the level, and the function is auto-set to be handled as a LEVEL BUTTON.
Entity createLevelButton(int level, std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center);

// Creates individual frames for the intro/conclusion story animations. These are treated as buttons, where the user clicks to go to next frame.
// - Use frameNumber to define which frame this is
// - MUST DEFINE MAX FRAME FOR STORY SEQUENCE TO TERMINATE
Entity createStoryFrame(int frameNumber, int maxFrame, std::string screen, TEXTURE_ASSET_ID texture);

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
Entity createBackgroundLayer();

// legacy
// the player
Entity createChicken(RenderSystem* renderer, vec2 position);
Entity createBall(b2WorldId worldId, vec2 startPos);
Entity createFireball(vec2 startPos);
Entity createConfetti(vec2 position);