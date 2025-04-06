#pragma once

#include <box2d/box2d.h>
#include <iostream>

b2BodyId create_vertical_wall(b2WorldId worldId, float x, float y, float height);
b2BodyId create_horizontal_wall(b2WorldId worldId, float x, float y, float width);

//////////////////////////////////////////////////////////////
// Should only be used by the load_level() funciton. Doesn't render textures, assume map textures are overlayed ontop.
//////////////////////////////////////////////////////////////
b2BodyId create_chain(b2WorldId worldId, std::vector<vec2> points, bool isLoop, std::vector<Entity>& linesArrayRef);

void spawnEnemyAtTile(b2WorldId worldId, bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 tile_position, ivec2 tile_movement_point_a, ivec2 tile_movement_point_b);
