#include <iostream>

#include "common.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "tinyECS/registry.hpp"
#include "tile.hpp"

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
b2BodyId create_vertical_wall(b2WorldId worldId, float x, float y, float height)
{
    float halfWidth = WALL_DEFAULT_THICKNESS / 2.0f;
    float halfHeight = height / 2.0f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{x, y};

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.1f;
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // Draw the vertical wall
    vec2 top = {x, y + halfHeight};
    vec2 bottom = {x, y - halfHeight};
    createLine(bottom, top);

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
b2BodyId create_horizontal_wall(b2WorldId worldId, float x, float y, float width)
{
    float halfWidth = width / 2.0f;
    float halfHeight = WALL_DEFAULT_THICKNESS / 2.0f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2{x, y};

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.friction = 0.1f;
    b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // Draw the horizontal wall
    vec2 left = {x - halfWidth, y};
    vec2 right = {x + halfWidth, y};
    createLine(left, right);

    return bodyId;
};

/**
 * @brief Renders a block at the center of the specified grid cell and creates a Box2D shape.
 *
 * This function creates a block using lines to represent its edges and also creates a Box2D
 * body with a polygon shape for physics interactions. The block is centered in the grid cell
 * specified by (tile_x, tile_y), where (0, 0) is the bottom-left grid cell.
 *
 * @param grid_position The x-coordinate and y-coordinate of the grid cell.
 * @param dimensions The full width and height of the block as a vec2.
 * @param textureId The ID of the texture to be used (currently unused).
 * @return b2BodyId The ID of the created Box2D body.
 */
b2BodyId create_single_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId)
{
    float halfWidth = GRID_CELL_WIDTH_PX / 2.0f;
    float halfHeight = GRID_CELL_HEIGHT_PX / 2.0f;

    // Calculate the center of the specified grid cell
    float centerX = (grid_position.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
    float centerY = (grid_position.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
    vec2 position = {centerX, centerY};

    //////////////////////////////////////////////////////////////
    // USE A STANDARD POLYGON SHAPE FOR THE PHYSICS BODY
    //////////////////////////////////////////////////////////////
    // Create Box2D body and shape
    // b2BodyDef bodyDef = b2DefaultBodyDef();
    // bodyDef.position = b2Vec2{ position.x, position.y };
    // b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // b2Polygon polygon = b2MakeBox(halfWidth, halfHeight);
    // b2ShapeDef shapeDef = b2DefaultShapeDef();
    // shapeDef.friction = 0.1f;  // Adjust friction as needed
    // b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

    // Calculate corner positions for rendering
    // vec2 topLeft = { position.x - halfWidth, position.y + halfHeight };
    // vec2 topRight = { position.x + halfWidth, position.y + halfHeight };
    // vec2 bottomRight = { position.x + halfWidth, position.y - halfHeight };
    // vec2 bottomLeft = { position.x - halfWidth, position.y - halfHeight };

    // Draw the block using lines
    // createLine(topLeft, topRight);         // Top edge
    // createLine(topRight, bottomRight);     // Right edge
    // createLine(bottomRight, bottomLeft);   // Bottom edge
    // createLine(bottomLeft, topLeft);       // Left edge

    //////////////////////////////////////////////////////////////
    // USE A CHAIN SHAPE FOR THE PHYSICS BODY
    //////////////////////////////////////////////////////////////
    std::vector<b2Vec2> objectSpaceVertices = TILE_GEOMETRY[textureId];

    // Translate vertices from object space to world space
    std::vector<b2Vec2> translatedVertices;
    translatedVertices.reserve(objectSpaceVertices.size());
    for (const auto &vertex : objectSpaceVertices)
    {
        translatedVertices.push_back(b2Vec2{
            vertex.x + grid_position.x * GRID_CELL_WIDTH_PX,
            vertex.y + grid_position.y * GRID_CELL_HEIGHT_PX,
        });
    }

    // Create a Box2D chain shape based on the translated vertices
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.count = translatedVertices.size();
    chainDef.points = translatedVertices.data();

    chainDef.isLoop = true;
    chainDef.friction = TERRAIN_DEFAULT_FRICTION;
    chainDef.restitution = TERRAIN_DEFAULT_RESTITUTION;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    b2CreateChain(bodyId, &chainDef);

    //////////////////////////////////////////////////////////////

    Entity entity = Entity();
    //// store a reference to the potentially re-used mesh object
    // Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    // registry.meshPtrs.emplace(entity, &mesh);

    // initialize the position, scale, and physics components
    auto &motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.position = position;
    motion.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);

    registry.renderRequests.insert(
        entity,
        {textureId,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});
    std::cout << position.x << ", " << position.y << std::endl;
    std::cout << b2Body_GetPosition(bodyId).x << ", " << b2Body_GetPosition(bodyId).y << std::endl;

    return bodyId;
}

void create_grapple_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId)
{
    float centerX = (grid_position.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
    float centerY = (grid_position.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
    vec2 position = {centerX, centerY};

    createGrapplePoint(worldId, position);

    // Outline
    Entity entity_grapple_outline = Entity();
    auto &grapple_outline_motion = registry.motions.emplace(entity_grapple_outline);
    grapple_outline_motion.position = vec2(position.x, position.y);
    grapple_outline_motion.scale = vec2(GRAPPLE_ATTACH_ZONE_RADIUS * 2, GRAPPLE_ATTACH_ZONE_RADIUS * 2);
    registry.renderRequests.insert(
        entity_grapple_outline,
        {TEXTURE_ASSET_ID::GRAPPLE_OUTLINE,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});
}

b2BodyId create_curve(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId)
{
    float halfWidth = GRID_CELL_WIDTH_PX / 2.0f;
    float halfHeight = GRID_CELL_HEIGHT_PX / 2.0f;

    // Calculate the center of the specified grid cell
    float centerX = (grid_position.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
    float centerY = (grid_position.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
    vec2 position = {centerX, centerY};

    //////////////////////////////////////////////////////////////
    // USE A CHAIN SHAPE FOR THE PHYSICS BODY
    //////////////////////////////////////////////////////////////
    std::vector<b2Vec2> objectSpaceVertices = TILE_GEOMETRY[textureId];

    // Translate vertices from object space to world space
    std::vector<b2Vec2> translatedVertices;
    translatedVertices.reserve(objectSpaceVertices.size());
    for (const auto &vertex : objectSpaceVertices)
    {
        translatedVertices.push_back(b2Vec2{
            vertex.x + grid_position.x * GRID_CELL_WIDTH_PX,
            vertex.y + grid_position.y * GRID_CELL_HEIGHT_PX,
        });
    }

    // Create a Box2D chain shape based on the translated vertices
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.count = translatedVertices.size();
    chainDef.points = translatedVertices.data();

    chainDef.isLoop = true;
    chainDef.friction = CURVED_RAMP_FRICTION;
    chainDef.restitution = CURVED_RAMP_RESTITUTION;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    b2CreateChain(bodyId, &chainDef);

    //////////////////////////////////////////////////////////////

    Entity entity = Entity();
    //// store a reference to the potentially re-used mesh object
    // Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    // registry.meshPtrs.emplace(entity, &mesh);

    // initialize the position, scale, and physics components
    auto &motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.position = position;
    motion.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);

    registry.renderRequests.insert(
        entity,
        {textureId,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});

    return bodyId;
}

b2BodyId create_block(b2WorldId worldId, vec2 start_tile, vec2 end_tile)
{
    // Calculate the minimum and maximum grid coordinates.
    float minX = std::min(start_tile.x, end_tile.x);
    float minY = std::min(start_tile.y, end_tile.y);
    float maxX = std::max(start_tile.x, end_tile.x);
    float maxY = std::max(start_tile.y, end_tile.y);

    // Compute the dimensions in tiles. We add 1 because a single tile wall (same x or y) should cover one cell.
    int widthTiles = static_cast<int>(maxX - minX) + 1;
    int heightTiles = static_cast<int>(maxY - minY) + 1;

    // Convert grid positions into world space boundaries.
    float worldStartX = minX * GRID_CELL_WIDTH_PX;
    float worldStartY = minY * GRID_CELL_HEIGHT_PX;
    float worldEndX = (minX + widthTiles) * GRID_CELL_WIDTH_PX;
    float worldEndY = (minY + heightTiles) * GRID_CELL_HEIGHT_PX;

    // Define the four vertices (clockwise order) for the chain shape.
    std::vector<b2Vec2> vertices;
    vertices.push_back(b2Vec2{worldStartX, worldStartY}); // Bottom-left
    vertices.push_back(b2Vec2{worldEndX, worldStartY});   // Bottom-right
    vertices.push_back(b2Vec2{worldEndX, worldEndY});     // Top-right
    vertices.push_back(b2Vec2{worldStartX, worldEndY});   // Top-left

    // Create a Box2D chain shape that loops (i.e. is closed)
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.count = vertices.size();
    chainDef.points = vertices.data();
    chainDef.isLoop = true;
    chainDef.friction = TERRAIN_DEFAULT_FRICTION;
    chainDef.restitution = TERRAIN_DEFAULT_RESTITUTION;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    b2CreateChain(bodyId, &chainDef);

    // For each grid cell inside the rectangle, calculate its center in world coordinates
    // and submit a render request with the specified texture.
    for (int row = static_cast<int>(minY); row < static_cast<int>(minY) + heightTiles; ++row)
    {
        for (int col = static_cast<int>(minX); col < static_cast<int>(minX) + widthTiles; ++col)
        {
            float centerX = (col * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
            float centerY = (row * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
            vec2 cellCenter = {centerX, centerY};

            Entity entity = Entity();
            auto &motion = registry.motions.emplace(entity);
            motion.angle = 0.f;
            motion.velocity = {0, 0};
            motion.position = cellCenter;
            motion.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);

            if (row == (minY + heightTiles - 1))
            {
                registry.renderRequests.insert(
                    entity,
                    {TEXTURE_ASSET_ID::SQUARE_TILE_2, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            }
            else
            {
                registry.renderRequests.insert(
                    entity,
                    {TEXTURE_ASSET_ID::SQUARE_TILE_1, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            }
        }
    }

    return bodyId;
}

void spawnEnemyAtTile(b2WorldId worldId, bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 tile_position, vec2 tile_movement_area)
{
    // Convert tile coordinates to pixel coordinates
    vec2 pixel_position = {
        tile_position.x * GRID_CELL_WIDTH_PX + (GRID_CELL_WIDTH_PX / 2.0f),
        tile_position.y * GRID_CELL_HEIGHT_PX + (GRID_CELL_HEIGHT_PX / 2.0f)};

    vec2 pixel_movement_area = {
        tile_movement_area.x * GRID_CELL_WIDTH_PX,
        tile_movement_area.y * GRID_CELL_HEIGHT_PX};

    // only create if predicate is true
    if (predicate)
    {
        // Create specified number of enemies by iterating
        for (int i = 0; i < quantity; i++)
        {
            // enemy created here
            createEnemy(worldId, pixel_position, enemy_type, pixel_movement_area);
        }
    }
}

void create_tutorial_tile(b2WorldId worldId, vec2 grid_position, TEXTURE_ASSET_ID textureId)
{
    float halfWidth = GRID_CELL_WIDTH_PX / 2.0f;
    float halfHeight = GRID_CELL_HEIGHT_PX / 2.0f;

    // Calculate the center of the specified grid cell
    float centerX = (grid_position.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
    float centerY = (grid_position.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
    vec2 position = {centerX, centerY};

    Entity entity = Entity();
    //// store a reference to the potentially re-used mesh object
    // Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    // registry.meshPtrs.emplace(entity, &mesh);

    auto &tutorial = registry.tutorialTiles.emplace(entity);

    // initialize the position, scale, and physics components
    auto &motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.position = position;
    motion.scale = vec2(GRID_CELL_WIDTH_PX - 2, GRID_CELL_HEIGHT_PX - 2);

    registry.renderRequests.insert(
        entity,
        {textureId,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});
}
