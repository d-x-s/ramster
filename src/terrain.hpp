#pragma once

#include <box2d/box2d.h>
#include <iostream>

b2BodyId create_vertical_wall(b2WorldId worldId, float x, float y, float height);
b2BodyId create_horizontal_wall(b2WorldId worldId, float x, float y, float width);
b2BodyId create_single_tile(b2WorldId worldId, vec2 position, TEXTURE_ASSET_ID textureId);
b2BodyId create_grapple_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId);
b2BodyId create_curve(b2WorldId worldId, vec2 position, TEXTURE_ASSET_ID textureId);
b2BodyId create_block(b2WorldId worldId, vec2 start_tile, vec2 end_tile);

void create_tutorial_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId);
void spawnEnemyAtTile(b2WorldId worldId, bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 tile_position, vec2 tile_movement_area);