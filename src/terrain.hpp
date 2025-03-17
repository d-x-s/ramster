#pragma once

#include <box2d/box2d.h>
#include <iostream>

b2BodyId create_vertical_wall(b2WorldId worldId, float x, float y, float height);
b2BodyId create_horizontal_wall(b2WorldId worldId, float x, float y, float width);
b2BodyId create_single_tile(b2WorldId worldId, vec2 position, TEXTURE_ASSET_ID textureId);
b2BodyId create_curve(b2WorldId worldId, vec2 position, TEXTURE_ASSET_ID textureId);
b2BodyId create_block(b2WorldId worldId, vec2 start_tile, vec2 end_tile);

//////////////////////////////////////////////////////////////
// Should only be used by the load_level() funciton. Doesn't render textures, assume map textures are overlayed ontop.
//////////////////////////////////////////////////////////////
b2BodyId create_chain(b2WorldId worldId, std::vector<vec2> points, bool isLoop, std::vector<Entity>& linesArrayRef);

void create_grapple_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId);
void create_tutorial_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId);
void spawnEnemyAtTile(b2WorldId worldId, bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 tile_position, vec2 tile_movement_area);