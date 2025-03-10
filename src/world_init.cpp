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
	bodyDef.position = b2Vec2{ BALL_INITIAL_POSITION_X, BALL_INITIAL_POSITION_Y };
	bodyDef.fixedRotation = false; // Allow rolling
	b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

	// Associate the body with a shape
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = BALL_DENSTIY;
	shapeDef.friction = BALL_FRICTION;
	shapeDef.restitution = BALL_RESTITUTION;
	b2Circle circle;
	circle.center = b2Vec2{ 0.0f, 0.0f };
	circle.radius = BALL_RADIUS;
	b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
	ball.bodyId = bodyId;
	ball.shapeId = shapeId;

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// Add motion & render request for ECS synchronization
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = vec2(BALL_INITIAL_POSITION_X, BALL_INITIAL_POSITION_Y);

	// The sprite is 64x64 pixels, and 1cm = 1pixel
	motion.scale = vec2(2 * circle.radius, 2 * circle.radius);

	// Associate player with camera
	auto& camera = registry.cameras.emplace(entity);
	camera.position = vec2(BALL_INITIAL_POSITION_X, BALL_INITIAL_POSITION_Y);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::RAMSTER_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

// This will create an enemy entity and place it on a random position in the map.
// INPUTS:
// - pos (x, y): position to spawn enemy
// - ENEMY_TYPES: type of enemy to spawn.
// - MOVEMENT AREA (min_x, max_x): activity radius of the enemy. set to (-1, -1) if you want enemy to move anywhere on the map.
Entity createEnemy(b2WorldId worldID, vec2 pos, ENEMY_TYPES enemy_type, vec2 movement_area) {

	// Determine enemy type-based characteristics here.
	// If the enemy is an obstacle then they will not be destructable. Can expand w/ more indestructable enemies.
	bool destructability = enemy_type == OBSTACLE ? false : true; 
	// Size of enemy. ENEMY_RADIUS is the standard size, and we'll change it for non-common enemies.
	float enemySize = ENEMY_RADIUS;
	if (enemy_type == OBSTACLE) {
		enemySize *= 2.5;
	}
	else if(enemy_type == SWARM) {
		enemySize *= 0.75;
	}
	// Bounciness of enemy. Maps onto box2D restitution. Common has the standard ENEMY_RESTITUTION.
	float enemyBounciness = ENEMY_RESTITUTION;
	if (enemy_type == OBSTACLE) {
		enemyBounciness = 0;
	}
	else if (enemy_type == SWARM) {
		enemyBounciness = 0.75;
	}
	// Weight of enemy, based on density. Common has default weight ENEMY_DENSITY
	float enemyWeight = ENEMY_DENSITY;
	if (enemy_type == OBSTACLE) {
		enemyWeight = 0.5;
	}
	else if (enemy_type == SWARM) {
		enemyWeight = 0.0005;
	}
	// Friction of enemy, which slows it down as it travels along a surface. Common has default friction ENEMY_FRICTION
	float enemyFriction = ENEMY_FRICTION;
	if (enemy_type == OBSTACLE) {
		enemyFriction = 0;
	}
	
	// Enemy entity
	Entity entity = Entity();

	// Add physics to enemy body
	PhysicsBody& enemyBody = registry.physicsBodies.emplace(entity);
	EnemyPhysics& enemy_physics = registry.enemyPhysics.emplace(entity);
	enemy_physics.isGrounded = false;

	// Add enemy component
	auto& enemy_registry = registry.enemies;
	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.enemyType = enemy_type;
	enemy.movement_area = movement_area;
	enemy.destructable = destructability;

	// Define a box2D body
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2{ pos[0], pos[1]};
	bodyDef.fixedRotation = true; // Fixed Rotation: true = no rolling, false = rolling.
	// Use `b2CreateBody()` instead of `world.CreateBody()`
	b2BodyId bodyId = b2CreateBody(worldID, &bodyDef);

	// Define shape properties using Box2D v3 functions
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = enemyWeight;
	shapeDef.friction = enemyFriction;
	shapeDef.restitution = enemyBounciness; 

	
	// Use `b2CreateCircleShape()` instead of `CreateFixture()`
	// We'll update the enemy hitbox later.
	b2Circle circle;
	circle.center = b2Vec2{ 0.0f, 0.0f };
	circle.radius = ENEMY_RADIUS;
	b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);

	enemyBody.bodyId = bodyId;
	enemyBody.shapeId = shapeId;

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// Add motion & render request for ECS synchronization
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = pos;

	float scale = circle.radius * 2;
	motion.scale = vec2(scale, scale);

	std::vector<TEXTURE_ASSET_ID> frames = { TEXTURE_ASSET_ID::FLOATER_1, TEXTURE_ASSET_ID::FLOATER_2, TEXTURE_ASSET_ID::FLOATER_3 };
	registry.renderRequests.insert(
		entity,
		{
			// TODO: Apply different textures to different enemy types.
			frames[0],                    // base apperance is just the first frame
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,                       // store all animation frames
			{},							              // no custom scale per frame
			true,						              // loop the animation
			200.0f,                       // frame time (ms)
			0.0f,                         // elapsed time
			0                             // current frame index
		}
	);
	// DEBUG std::cout << "Inserted render request for enemy.\n";

	return entity;
}
Entity createGrapplePoint(b2WorldId worldId, vec2 position){
	Entity entity = Entity();

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = b2Vec2{ position.x, position.y};

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();

	//Disable collisions
	shapeDef.filter.maskBits = 0x0000;
	shapeDef.isSensor = true; 

	b2Circle circle;
	circle.center = b2Vec2{ 0.0f, 0.0f };
    circle.radius = 0.2f;

	b2CreateCircleShape(bodyId, &shapeDef, &circle);

	PhysicsBody& grappleBody = registry.physicsBodies.emplace(entity);
    grappleBody.bodyId = bodyId;

	GrapplePoint& grapplePoint = registry.grapplePoints.emplace(entity);
	grapplePoint.position = position;
	grapplePoint.active = false;
	grapplePoint.bodyId = bodyId;
	
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = vec2(64.0f, 64.0f);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GRAPPLE_POINT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createGrapple(b2WorldId worldId, b2BodyId ballBodyId, b2BodyId grappleBodyId, float distance) {
    Entity entity = Entity();

    // Create distance joint between ball and grapple Point
    b2DistanceJointDef djd = b2DefaultDistanceJointDef();
    djd.bodyIdA = ballBodyId;
    djd.bodyIdB = grappleBodyId;
    djd.length = distance;
    djd.collideConnected = false;
    djd.maxLength = 450.0f;
	djd.minLength = 100.0f;

    b2JointId jointId = b2CreateDistanceJoint(worldId, &djd);

    // Store grapple data
    Grapple& grapple = registry.grapples.emplace(entity);
    grapple.jointId = jointId;
    grapple.ballBodyId = ballBodyId;
    grapple.grappleBodyId = grappleBodyId;

    // Create line entity to visualize grapple
    b2Vec2 ballPos = b2Body_GetPosition(ballBodyId);
    b2Vec2 grapplePos = b2Body_GetPosition(grappleBodyId);

    Entity lineEntity = createLine(vec2(ballPos.x, ballPos.y), vec2(grapplePos.x, grapplePos.y));
    grapple.lineEntity = lineEntity;  // Store the line entity for updates

    return entity;
}

void removeGrapple(){
	for (Entity& grapple_entity : registry.grapples.entities) {
		Grapple& grapple = registry.grapples.get(grapple_entity);
		b2DestroyJoint(grapple.jointId);
		registry.remove_all_components_of(grapple_entity);

		if (registry.lines.has(grapple.lineEntity)) {
        	registry.remove_all_components_of(grapple.lineEntity);
    	}
	}
	
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! {{{ OK }}} TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	GridLine& gridLine = registry.gridLines.emplace(entity);
	gridLine.start_pos = start_pos;
	gridLine.end_pos = start_pos + end_pos;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);
	
	registry.colors.insert(entity, vec3(0.0f, 1.0f, 0.0f)); 
	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! line segments between arbitrary points
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	Line& line = registry.lines.emplace(entity);
	line.start_pos = start_pos;
	line.end_pos = end_pos;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	registry.colors.insert(entity, vec3(1.0f, 1.0f, 1.0f));
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
			{},							              // no custom scale per frame
			true,						              // loop the animation
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
			TEXTURE_ASSET_ID::RAMSTER_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

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