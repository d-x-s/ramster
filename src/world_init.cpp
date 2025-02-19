#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

Entity createBall(b2WorldId worldId)
{
	Entity entity = Entity();

	// Add physics and player components
	PhysicsBody& ball = registry.physicsBodies.emplace(entity);
	PlayerPhysics& ball_physics = registry.playerPhysics.emplace(entity);
	ball_physics.isGrounded = false;

	auto& player_registry = registry.players;
	Player& player = registry.players.emplace(entity);

	// Define a dynamic body
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2{ 100.0f, 100.0f };
	bodyDef.fixedRotation = false; // Allow rolling

	// Use `b2CreateBody()` instead of `world.CreateBody()`
	b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
	std::cout << "Dynamic body created at position ("
		<< bodyDef.position.x << ", " << bodyDef.position.y << ")\n";

	// Define shape properties using Box2D v3 functions
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = BALL_DENSTIY;
	shapeDef.friction = BALL_FRICTION;
	shapeDef.restitution = BALL_RESTITUTION; // Higher restitution makes it bouncy

	// Use `b2CreateCircleShape()` instead of `CreateFixture()`
	b2Circle circle;
	circle.center = b2Vec2{ 0.0f, 0.0f };
	circle.radius = 0.35f;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
	std::cout << "Dynamic fixture added with radius 0.5, density=1.0, friction=0.3, restitution=0.8 (bouncy).\n";

	ball.bodyId = bodyId;

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// Add motion & render request for ECS synchronization
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = vec2(100.0f, 100.0f);

	float scale = circle.radius * 100.f;
	motion.scale = vec2(scale, scale);
	std::cout << "world_init.cpp: createBall: Added motion component to ball.\n";

	// Associate player with camera
	auto& camera = registry.cameras.emplace(entity);
	camera.position = vec2(100.0f, 100.0f);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	std::cout << "Inserted render request.\n";

	return entity;
}

// This will create an enemy entity and place it on a random position in the map.
Entity createEnemy(b2WorldId worldID, vec2 pos) {

	//figure out x and y coordinates
	float min_x = 0;
	float max_x = WINDOW_WIDTH_PX * 3.0; //this is also the room width
	float min_y = 0;
	float max_y = WINDOW_HEIGHT_PX; // this is also room height


	Entity entity = Entity();

	// Add physics and enemy components
	PhysicsBody& enemy = registry.physicsBodies.emplace(entity);
	EnemyPhysics& enemy_physics = registry.enemyPhysics.emplace(entity);
	enemy_physics.isGrounded = false;

	auto& enemy_registry = registry.enemies;
	Enemy& enemy = registry.enemies.emplace(entity);

	// Define a dynamic body
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2{ pos[0], pos[1]};
	// commenting this out should disable rolling...?  bodyDef.fixedRotation = false; // Allow rolling

	// Use `b2CreateBody()` instead of `world.CreateBody()`
	b2BodyId bodyId = b2CreateBody(worldID, &bodyDef);
	std::cout << "Dynamic body ENEMY created at position ("
		<< bodyDef.position.x << ", " << bodyDef.position.y << ")\n";

	// Define shape properties using Box2D v3 functions
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = ENEMY_DENSITY;
	shapeDef.friction = ENEMY_FRICTION;
	shapeDef.restitution = ENEMY_RESTITUTION; 

	// Use `b2CreateCircleShape()` instead of `CreateFixture()`
	b2Circle circle;
	circle.center = b2Vec2{ 0.0f, 0.0f };
	circle.radius = 0.35f;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
	std::cout << "Dynamic fixture added with radius 0.5, density=1.0, friction=0.1, restitution=0.1 (bouncy).\n";

	enemy.bodyId = bodyId;

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// Add motion & render request for ECS synchronization
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = pos;

	float scale = circle.radius * 100.f;
	motion.scale = vec2(scale, scale);
	std::cout << "world_init.cpp: createEnemy: Added motion component to enemy.\n";

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BLUE_INVADER_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	std::cout << "Inserted render request for enemy.\n";

	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! {{{ OK }}} TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	// {{{ OK }}} TODO A1: create a gridLine component
	GridLine& gridLine = registry.gridLines.emplace(entity);
	gridLine.start_pos = start_pos;
	gridLine.end_pos = start_pos + end_pos;

	// re-use the "DEBUG_LINE" renderRequest
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);
	
	// {{{ OK }}} TODO A1: grid line color (choose your own color, RGB gray)
	registry.colors.insert(entity, vec3(0.5f, 0.5f, 0.5f)); 

	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! {{{ OK }}} TODO A1: createInvader?
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createInvader(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();

	// invader
	Invader& invader = registry.invaders.emplace(entity);

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// {{{ OK }}} TODO A1: initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;

	// resize, set scale to negative if you want to make it face the opposite way
	// motion.scale = vec2({ -INVADER_BB_WIDTH, INVADER_BB_WIDTH });
	motion.scale = vec2({ INVADER_BB_WIDTH, INVADER_BB_HEIGHT });

	// create an (empty) Bug component to be able to refer to all bug (outdated?)
	registry.eatables.emplace(entity);

	// randomly select invader color, corresponding animation frames, and set health
	std::vector<TEXTURE_ASSET_ID> frames;
	int random_choice = rand() % 4;

	switch (random_choice) {
	case 0:
		frames = { TEXTURE_ASSET_ID::RED_INVADER_1, TEXTURE_ASSET_ID::RED_INVADER_2, TEXTURE_ASSET_ID::RED_INVADER_3 };
		invader.health = INVADER_HEALTH_RED;
		motion.velocity = { INVADER_VELOCITY_RED, 0 };
		break;
	case 1:
		frames = { TEXTURE_ASSET_ID::BLUE_INVADER_1, TEXTURE_ASSET_ID::BLUE_INVADER_2, TEXTURE_ASSET_ID::BLUE_INVADER_3 };
		invader.health = INVADER_HEALTH_BLUE;
		motion.velocity = { INVADER_VELOCITY_BLUE, 0 };
		break;
	case 2:
		frames = { TEXTURE_ASSET_ID::GREEN_INVADER_1, TEXTURE_ASSET_ID::GREEN_INVADER_2, TEXTURE_ASSET_ID::GREEN_INVADER_3 };
		invader.health = INVADER_HEALTH_GREEN;
		motion.velocity = { INVADER_VELOCITY_GREEN, 0 };
		break;
	case 3:
		frames = { TEXTURE_ASSET_ID::GREY_INVADER_1, TEXTURE_ASSET_ID::GREY_INVADER_2, TEXTURE_ASSET_ID::GREY_INVADER_3 };
		invader.health = INVADER_HEALTH_GREY;
		motion.velocity = { INVADER_VELOCITY_GREY, 0 };
		break;
	}

	// insert render request with animation frames
	registry.renderRequests.insert(
		entity,
		{
			frames[0],                    // base apperance is just the first frame
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,                       // store all animation frames
			{},							  // no custom scale per frame
			true,						  // loop the animation
			200.0f,                       // frame time (ms)
			0.0f,                         // elapsed time
			0                             // current frame index
		}
	);

	return entity;
}

Entity createExplosion(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();

	// explosion
	Explosion& invader = registry.explosions.emplace(entity);

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// {{{ OK }}} TODO A1: initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	// resize, set scale to negative if you want to make it face the opposite way
	motion.scale = vec2({ EXPLOSION_BB_WIDTH, EXPLOSION_BB_HEIGHT });

	// create an (empty) Bug component to be able to refer to all bug (outdated?)
	registry.eatables.emplace(entity);

	// randomly select invader color, corresponding animation frames, and set health
	std::vector<TEXTURE_ASSET_ID> frames = { TEXTURE_ASSET_ID::EXPLOSION_1, TEXTURE_ASSET_ID::EXPLOSION_2, TEXTURE_ASSET_ID::EXPLOSION_3 };
	std::vector<float> scales = { 0.75f, 1.0f, 1.25f };

	// insert render request with animation frames
	registry.renderRequests.insert(
		entity,
		{
			frames[0],                    // base apperance is just the first frame
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,                       // store all animation frames
			scales,						  // assign custom scale per frame
			false,						  // do not loop the animation
			200.0f,                       // frame time (ms)
			0.0f,                         // elapsed time
			0                             // current frame index
		}
	);

	return entity;
}

Entity createTower(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto& t = registry.towers.emplace(entity);
	t.range = (float)WINDOW_WIDTH_PX / (float)GRID_CELL_WIDTH_PX;
	t.timer_ms = TOWER_TIMER_MS;	// arbitrary for now

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;	// A1-TD: CK: rotate to the left 180 degrees to fix orientation
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	std::cout << "INFO: tower position: " << position.x << ", " << position.y << std::endl;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -TOWER_BB_WIDTH, TOWER_BB_HEIGHT });

	// create an (empty) Tower component to be able to refer to all towers
	registry.deadlys.emplace(entity);
  
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TOWER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

void removeTower(vec2 position) {
	// remove any towers at this position
	for (Entity& tower_entity : registry.towers.entities) {
		// get each tower's position to determine it's row
		const Motion& tower_motion = registry.motions.get(tower_entity);
		
		if (tower_motion.position.y == position.y) {
			// remove this tower
			registry.remove_all_components_of(tower_entity);
			std::cout << "tower removed" << std::endl;
		}
	}
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! {{{ OK }}} TODO A1: create a new projectile w/ pos, size, & velocity
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();

	// {{{ OK }}} TODO: projectile
	auto& p = registry.projectiles.emplace(entity);
	p.damage = PROJECTILE_DAMAGE;
	 
	// {{{ OK }}} TODO: motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = velocity;
	motion.angle = 0.f;
	motion.scale = size;

	// {{{ OK }}} TODO: renderRequests
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{
			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

// LEGACY
Entity createChicken(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::CHICKEN);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;
	motion.scale.y *= -1; // point front to the right

	// create an (empty) Chicken component to be able to refer to all towers
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::CHICKEN
		}
	);

	return entity;
}