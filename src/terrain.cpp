#include <iostream>

#include "common.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "tinyECS/registry.hpp"

/**
 * @brief Creates a vertical wall centered at the specified position.
 *
 * This function creates a static vertical wall with the given height and a fixed width of 1 unit,
 * centered at the specified (x, y) position.
 *
 * @param worldId The ID of the Box2D world where the wall will be created.
 * @param x The x-coordinate of the wall's center position.
 * @param y The y-coordinate of the wall's center position.
 * @param height The full height of the wall.
 * @return b2BodyId The ID of the created wall body in the Box2D world.
 */
b2BodyId create_vertical_wall(b2WorldId worldId, float x, float y, float height) {
    float halfWidth = WALL_DEFAULT_THICKNESS / 2.0f;
    float halfHeight = height / 2.0f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{ x, y };

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.1f;
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // create PhysicsBody component
    Entity entity = Entity();
    PhysicsBody& physicsBody = registry.physicsBodies.emplace(entity);
    physicsBody.bodyId = bodyId;

    return bodyId;
};

/**
 * @brief Creates a horizontal wall centered at the specified position.
 *
 * This function creates a static horizontal wall with the given width and a fixed height of 1 unit,
 * centered at the specified (x, y) position.
 *
 * @param worldId The ID of the Box2D world where the wall will be created.
 * @param x The x-coordinate of the wall's center position.
 * @param y The y-coordinate of the wall's center position.
 * @param width The full width of the wall.
 * @return b2BodyId The ID of the created wall body in the Box2D world.
 */
b2BodyId create_horizontal_wall(b2WorldId worldId, float x, float y, float width) {
    float halfWidth = width / 2.0f;
    float halfHeight = WALL_DEFAULT_THICKNESS / 2.0f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{ x, y };

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.1f;
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // create PhysicsBody component
    Entity entity = Entity();
	PhysicsBody& physicsBody = registry.physicsBodies.emplace(entity);
	physicsBody.bodyId = bodyId;

    return bodyId;
};

b2BodyId create_chain(b2WorldId worldId, std::vector<vec2> points, bool isLoop, std::vector<Entity>& linesArrayRef) {
    // convert the coords to world space.
    std::vector<b2Vec2> translatedVertices;
    translatedVertices.reserve(points.size());
    for (const auto& vertex : points) {
        translatedVertices.push_back(b2Vec2{
            vertex.x * TILED_TO_GRID_PIXEL_SCALE,
            vertex.y * TILED_TO_GRID_PIXEL_SCALE,
            });
    }

    // Create a Box2D chain shape based on the translated vertices
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.count = translatedVertices.size();
    chainDef.points = translatedVertices.data();

    chainDef.isLoop = isLoop;
    chainDef.friction = TERRAIN_DEFAULT_FRICTION;
    chainDef.restitution = TERRAIN_DEFAULT_RESTITUTION;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    b2CreateChain(bodyId, &chainDef);

    // create physicsBody component to track the terrain shapes.
    Entity entity = Entity();
    PhysicsBody& pb = registry.physicsBodies.emplace(entity);
	  pb.bodyId = bodyId;

    return bodyId;
}

void spawnEnemyAtTile(b2WorldId worldId, bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 tile_position, ivec2 tile_movement_point_a, ivec2 tile_movement_point_b) {
    // Convert tile coordinates to pixel coordinates
    vec2 pixel_position = {
        tile_position.x * GRID_CELL_WIDTH_PX + (GRID_CELL_WIDTH_PX / 2.0f),
        tile_position.y * GRID_CELL_HEIGHT_PX + (GRID_CELL_HEIGHT_PX / 2.0f)
    };

    vec2 pixel_movement_area_bottom_left = {
        tile_movement_point_a.x * GRID_CELL_WIDTH_PX,
        tile_movement_point_a.y * GRID_CELL_HEIGHT_PX
    };

    vec2 pixel_movement_area_top_right = {
        tile_movement_point_b.x * GRID_CELL_WIDTH_PX,
        tile_movement_point_b.y * GRID_CELL_HEIGHT_PX
    };

    // only create if predicate is true
    if (predicate) {
        // Create specified number of enemies by iterating
        for (int i = 0; i < quantity; i++) {
            // enemy created here
            createEnemy(worldId, pixel_position, enemy_type, pixel_movement_area_bottom_left, pixel_movement_area_top_right);
        }
    }
}



