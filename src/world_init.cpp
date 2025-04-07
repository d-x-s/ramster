#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>


// Creates tracker component for current screen
Entity createCurrentScreen() {
	Entity entity = Entity();

	CurrentScreen& currentScreen = registry.currentScreen.emplace(entity);

	return entity;
}

// TODO: Port createScreen here.
Entity createScreenElement(std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center) {

	Entity entity = Entity(); 

	// Make screen element
	ScreenElement& screenElement = registry.screenElements.emplace(entity);
	screenElement.screen = screen;

	// Configure size (for some reason we need motion to render?)
	auto& motion = registry.motions.emplace(entity);
	motion.position = vec2(WORLD_WIDTH_PX / 2, WORLD_HEIGHT_PX / 4); // This is a placeholder. Actual position computed during runtime.
	motion.scale = vec2(width_px, height_px); // Scaled to defined width/height

	// Attach camera to center screen
	Entity camera = registry.cameras.entities[0];
	screenElement.camera = camera;

	// Set position relative to camera center
	screenElement.position = pos_relative_center;

	// Define boundaries based around center (so, center wrapped by boundary walls)
	screenElement.boundaries = vec4(screenElement.position.x - (width_px / 2), screenElement.position.y - (height_px / 2),
									screenElement.position.x + (width_px / 2), screenElement.position.y + (height_px / 2));
	// LLNOTE: IDK how it ended up like this but after much trial and error these are the values that I needed for those buttons to get hitboxes right.
	/*
	screenElement.boundaries = vec4(screenElement.position.x - (width_px / 3.175), screenElement.position.y - (height_px / 2.05),
									screenElement.position.x + (width_px / 3.225), screenElement.position.y + (height_px / 2.85));
	*/

	// Add to render requests with specified texture
	registry.renderRequests.insert(
		entity,
		{
			texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;

}


// Makes button and renders as screen element.
Entity createButton(std::string function, std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center) {

	Entity buttonElement = createScreenElement(screen, texture, width_px, height_px, pos_relative_center);
	Button& button = registry.buttons.emplace(buttonElement);
	button.function = function;

	return buttonElement;
}


// Same as create button, but ties a level component onto it.
Entity createLevelButton(int level, std::string screen, TEXTURE_ASSET_ID texture, int width_px, int height_px, vec2 pos_relative_center) {

	Entity buttonElement = createButton("LEVEL BUTTON", screen, texture, width_px, height_px, pos_relative_center);
	Level& levelComponent = registry.levels.emplace(buttonElement);
	levelComponent.level = level;

	return buttonElement;
}


Entity createStoryFrame(int frameNumber, int maxFrame, std::string screen, TEXTURE_ASSET_ID texture) {

	Entity buttonElement = createButton("STORY FRAME BUTTON", screen, texture, 1366, 768, vec2(0, 0));
	StoryFrame& storyFrame = registry.storyFrames.emplace(buttonElement);
	storyFrame.frame = frameNumber;
	storyFrame.max_frame = maxFrame;

	return buttonElement;
}


Entity createBall(b2WorldId worldId, vec2 startPos)
{
	Entity mainEntity = Entity();

	// Add physics and player components
	PhysicsBody& ball = registry.physicsBodies.emplace(mainEntity);
	PlayerPhysics& ball_physics = registry.playerPhysics.emplace(mainEntity);
	ball_physics.isGrounded = false;

	Player& player = registry.players.emplace(mainEntity);
	player.isCurrentlyFlamming = false;
	player.isCurrentlyRolling = false;
	player.enemiesRecentlyDestroyed = 0;
	player.voicelineProbability = 0;
	player.lastVoicelineTime = std::chrono::steady_clock::now();

	// Define a dynamic body
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2{ startPos.x, startPos.y };
	bodyDef.fixedRotation = false;
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

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// Add motion & render request
	auto& motion = registry.motions.emplace(mainEntity);
	motion.angle = 0.f;
	motion.position = startPos;
	motion.scale = vec2(2 * circle.radius, 2 * circle.radius);

	auto& camera = registry.cameras.emplace(mainEntity);
	camera.position = startPos;

	// Helpers to create additional visual layer entities
	auto createRotatableLayer = [&](TEXTURE_ASSET_ID textureId, EFFECT_ASSET_ID effectId, std::string layer) {
		Entity ballVisualEntity = Entity();

		if (layer == "front") {
			registry.playerTopLayer.emplace(ballVisualEntity);
		} 
		else if (layer == "middle") {
			registry.playerMidLayer.emplace(ballVisualEntity);
		}
		else {
			registry.playerBottomLayer.emplace(ballVisualEntity);
		}

		auto& m = registry.motions.emplace(ballVisualEntity);
		m.angle = 0.f;
		m.position = startPos;
		m.scale = vec2(2 * circle.radius, 2 * circle.radius);

		registry.playerRotatableLayers.emplace(ballVisualEntity);

		registry.renderRequests.insert(
			ballVisualEntity,
			{
				textureId,
				effectId,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	};

	auto createRamsterRunLayer = [&]() {
		Entity ramsterVisualEntity = Entity();

		auto& m = registry.motions.emplace(ramsterVisualEntity);
		m.angle = 0.f;
		m.position = startPos;
		m.scale = vec2(2 * circle.radius, 2 * circle.radius);

		registry.playerMidLayer.emplace(ramsterVisualEntity);
		registry.playerNonRotatableLayers.emplace(ramsterVisualEntity);
		registry.runAnimations.emplace(ramsterVisualEntity);

		std::vector<TEXTURE_ASSET_ID> frames;
		frames = {
			TEXTURE_ASSET_ID::RAMSTER_RUN_0,
			TEXTURE_ASSET_ID::RAMSTER_RUN_1,
			TEXTURE_ASSET_ID::RAMSTER_RUN_2,
			TEXTURE_ASSET_ID::RAMSTER_RUN_3,
			TEXTURE_ASSET_ID::RAMSTER_RUN_4,
			TEXTURE_ASSET_ID::RAMSTER_RUN_5,
			TEXTURE_ASSET_ID::RAMSTER_RUN_6,
			TEXTURE_ASSET_ID::RAMSTER_RUN_7,
		};

		registry.renderRequests.insert(
			ramsterVisualEntity,
			{
				frames[0],
				EFFECT_ASSET_ID::RAMSTER,
				GEOMETRY_BUFFER_ID::SPRITE,
				frames,
				{},
				true,
				true,
				100.0f,
				0.0f,
				0
			}
		);
	};

	auto createRamsterIdleLayer = [&]() {
		Entity ramsterVisualEntity = Entity();

		auto& m = registry.motions.emplace(ramsterVisualEntity);
		m.angle = 0.f;
		m.position = startPos;
		m.scale = vec2(2 * circle.radius, 2 * circle.radius);

		registry.playerMidLayer.emplace(ramsterVisualEntity);
		registry.playerNonRotatableLayers.emplace(ramsterVisualEntity);
		registry.idleAnimations.emplace(ramsterVisualEntity);

		std::vector<TEXTURE_ASSET_ID> frames;
		frames = {
			TEXTURE_ASSET_ID::RAMSTER_IDLE_0,
			TEXTURE_ASSET_ID::RAMSTER_IDLE_1,
			TEXTURE_ASSET_ID::RAMSTER_IDLE_2,
			TEXTURE_ASSET_ID::RAMSTER_IDLE_3,
			TEXTURE_ASSET_ID::RAMSTER_IDLE_4,
			TEXTURE_ASSET_ID::RAMSTER_IDLE_5,
		};

		registry.renderRequests.insert(
			ramsterVisualEntity,
			{
				frames[0],
				EFFECT_ASSET_ID::RAMSTER,
				GEOMETRY_BUFFER_ID::SPRITE,
				frames,
				{},
				true,
				false,
				200.0f,
				0.0f,
				0
			}
		);
	};

	// ========================================================================================================
	// create entities for Ball and Ramster layers
	// ========================================================================================================
	// Glass ball (rotatable)
	createRotatableLayer(TEXTURE_ASSET_ID::RAMSTER_GLASS_WALL, EFFECT_ASSET_ID::TRANSLUCENT, "front");
	createRotatableLayer(TEXTURE_ASSET_ID::RAMSTER_GLASS_BACK, EFFECT_ASSET_ID::TEXTURED, "back");
	createRotatableLayer(TEXTURE_ASSET_ID::RAMSTER_GLASS_FRONT, EFFECT_ASSET_ID::TEXTURED, "front");

	// Ramster (non-rotatable)
	createRamsterRunLayer();
	createRamsterIdleLayer();

	// ========================================================================================================
	// create new entity for fireball fx
	// ========================================================================================================
	createFireball(startPos);

	return mainEntity;
}

Entity createConfetti(vec2 position) {
	Entity entity = Entity();

	// Add motion
	auto& confetti_motion = registry.motions.emplace(entity);
	confetti_motion.scale = vec2(700.f, 700.f);
	confetti_motion.angle = 0.f;
	confetti_motion.position = position;

	// render request and frames
	std::vector<TEXTURE_ASSET_ID> frames;

	frames = {
	TEXTURE_ASSET_ID::CONFETTI_0,
	TEXTURE_ASSET_ID::CONFETTI_1,
	TEXTURE_ASSET_ID::CONFETTI_2,
	TEXTURE_ASSET_ID::CONFETTI_3,
	TEXTURE_ASSET_ID::CONFETTI_4,
	TEXTURE_ASSET_ID::CONFETTI_5,
	TEXTURE_ASSET_ID::CONFETTI_6,
	TEXTURE_ASSET_ID::CONFETTI_7,
	TEXTURE_ASSET_ID::CONFETTI_8,
	TEXTURE_ASSET_ID::CONFETTI_9,
	TEXTURE_ASSET_ID::CONFETTI_10,
	TEXTURE_ASSET_ID::CONFETTI_11,
	TEXTURE_ASSET_ID::CONFETTI_12,
	TEXTURE_ASSET_ID::CONFETTI_13,
	TEXTURE_ASSET_ID::CONFETTI_14,
	TEXTURE_ASSET_ID::CONFETTI_15,
	TEXTURE_ASSET_ID::CONFETTI_16,
	TEXTURE_ASSET_ID::CONFETTI_17,
	TEXTURE_ASSET_ID::CONFETTI_18,
	TEXTURE_ASSET_ID::CONFETTI_19,
	TEXTURE_ASSET_ID::CONFETTI_20,
	TEXTURE_ASSET_ID::CONFETTI_21,
	TEXTURE_ASSET_ID::CONFETTI_22,
	TEXTURE_ASSET_ID::CONFETTI_23,
	TEXTURE_ASSET_ID::CONFETTI_24,
	TEXTURE_ASSET_ID::CONFETTI_25,
	TEXTURE_ASSET_ID::CONFETTI_26,
	TEXTURE_ASSET_ID::CONFETTI_27,
	TEXTURE_ASSET_ID::CONFETTI_28,
	TEXTURE_ASSET_ID::CONFETTI_29,
	TEXTURE_ASSET_ID::CONFETTI_30,
	TEXTURE_ASSET_ID::CONFETTI_31,
	TEXTURE_ASSET_ID::CONFETTI_32,
	TEXTURE_ASSET_ID::CONFETTI_33,
	TEXTURE_ASSET_ID::CONFETTI_34,
	TEXTURE_ASSET_ID::CONFETTI_35,
	TEXTURE_ASSET_ID::CONFETTI_36,
	TEXTURE_ASSET_ID::CONFETTI_37,
	TEXTURE_ASSET_ID::CONFETTI_38,
	TEXTURE_ASSET_ID::CONFETTI_39,
	TEXTURE_ASSET_ID::CONFETTI_40,
	TEXTURE_ASSET_ID::CONFETTI_41,
	TEXTURE_ASSET_ID::CONFETTI_42,
	TEXTURE_ASSET_ID::CONFETTI_43,
	TEXTURE_ASSET_ID::CONFETTI_44,
	TEXTURE_ASSET_ID::CONFETTI_45,
	TEXTURE_ASSET_ID::CONFETTI_46,
	TEXTURE_ASSET_ID::CONFETTI_47,
	TEXTURE_ASSET_ID::CONFETTI_48,
	TEXTURE_ASSET_ID::CONFETTI_49,
	TEXTURE_ASSET_ID::CONFETTI_50,
	TEXTURE_ASSET_ID::CONFETTI_51,
	TEXTURE_ASSET_ID::CONFETTI_52,
	TEXTURE_ASSET_ID::CONFETTI_53,
	TEXTURE_ASSET_ID::CONFETTI_54,
	TEXTURE_ASSET_ID::CONFETTI_55,
	TEXTURE_ASSET_ID::CONFETTI_56,
	TEXTURE_ASSET_ID::CONFETTI_57,
	TEXTURE_ASSET_ID::CONFETTI_58,
	};

	registry.renderRequests.insert(
		entity,
		{
			frames[29],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,
			{},
			true,
			true,
			30.0f,
			0.0f,
			29
		}
	);

	return entity;
}

Entity createFireball(vec2 startPos) {

	Entity entity = Entity();

	auto& fireball = registry.fireballs.emplace(entity);

	// Add motion
	auto& fireball_motion = registry.motions.emplace(entity);
	fireball_motion.scale = vec2(200.f, 75.f);
	fireball_motion.angle = 0.f;
	fireball_motion.position = startPos;

	std::vector<TEXTURE_ASSET_ID> frames;

	frames = { 
		TEXTURE_ASSET_ID::FIREBALL_0, 
		TEXTURE_ASSET_ID::FIREBALL_1,
		TEXTURE_ASSET_ID::FIREBALL_2,
		TEXTURE_ASSET_ID::FIREBALL_3,
		TEXTURE_ASSET_ID::FIREBALL_4,
		TEXTURE_ASSET_ID::FIREBALL_5,
		TEXTURE_ASSET_ID::FIREBALL_6,
		TEXTURE_ASSET_ID::FIREBALL_7,
		TEXTURE_ASSET_ID::FIREBALL_8,
		TEXTURE_ASSET_ID::FIREBALL_9,
		TEXTURE_ASSET_ID::FIREBALL_10,
		TEXTURE_ASSET_ID::FIREBALL_11
	};

	registry.renderRequests.insert(
		entity,
		{
			frames[0],
			EFFECT_ASSET_ID::FIREBALL,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,
			{},
			true,
			false,
			60.0f,
			0.0f,
			0
		}
	);

	return entity;
}

// This will create an enemy entity and place it on a random position in the map.
// INPUTS:
// - pos (x, y): position to spawn enemy
// - ENEMY_TYPES: type of enemy to spawn.
// - MOVEMENT AREA (min_x, max_x): activity radius of the enemy. set to (-1, -1) if you want enemy to move anywhere on the map.
Entity createEnemy(b2WorldId worldID, vec2 pos, ENEMY_TYPES enemy_type, vec2 movement_range_point_a, vec2 movement_range_point_b) {

	// Determine enemy type-based characteristics here.
 
	// If the enemy is an obstacle then they will not be destructable. Can expand w/ more indestructable enemies.
	bool destructability = enemy_type == OBSTACLE ? false : true; 
	// Size of enemy. ENEMY_RADIUS is the standard size, and we'll change it for non-common enemies.
	float enemySize = ENEMY_RADIUS;
	if (enemy_type == OBSTACLE) {
		enemySize *= 1.5;
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
		enemyBounciness = 0.5;
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
	// Whether the enemy is affected by gravity, applied using gravity scaling. Only common enemies have gravity.
	float enemyGravityScaling = 0;
	if (enemy_type == COMMON) {
		enemyGravityScaling = 1;
	}
	

	// Add enemy to ECS
	
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
	enemy.movement_area_point_a = movement_range_point_a;
	enemy.movement_area_point_b = movement_range_point_b;
	enemy.destructable = destructability;


	// Make Box2D body for enemy
	
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
	circle.radius = enemySize; // Simple way to change size of enemies based on their type. Adjust int multiplier in ENEMY_TYPES to change this.
	b2CreateCircleShape(bodyId, &shapeDef, &circle); // this is the hitbox

	enemyBody.bodyId = bodyId;

	b2Body_SetAngularDamping(bodyId, BALL_ANGULAR_DAMPING);

	// This changes the effect of gravity on an enemy.
	b2Body_SetGravityScale(bodyId, enemyGravityScaling);


	// Add motion & render request for ECS synchronization
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = pos;

	float scale = circle.radius * 3;
	motion.scale = vec2(scale, scale);

	std::vector<TEXTURE_ASSET_ID> frames;

	if (enemy_type == ENEMY_TYPES::COMMON) {
		frames = { TEXTURE_ASSET_ID::COMMON_1, TEXTURE_ASSET_ID::COMMON_2, TEXTURE_ASSET_ID::COMMON_3, TEXTURE_ASSET_ID::COMMON_4, TEXTURE_ASSET_ID::COMMON_5 };
	}
	else if (enemy_type == ENEMY_TYPES::SWARM)
	{
		frames = { TEXTURE_ASSET_ID::SWARM_1, TEXTURE_ASSET_ID::SWARM_2, TEXTURE_ASSET_ID::SWARM_3, TEXTURE_ASSET_ID::SWARM_4 };
	}
	else if (enemy_type == ENEMY_TYPES::OBSTACLE)
	{
		frames = { TEXTURE_ASSET_ID::OBSTACLE_1, TEXTURE_ASSET_ID::OBSTACLE_2, TEXTURE_ASSET_ID::OBSTACLE_3, TEXTURE_ASSET_ID::OBSTACLE_4 };
	}

	registry.renderRequests.insert(
		entity,
		{
			frames[0],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			frames,
			{},
			true,
			true,
			200.0f,                       
			0.0f,                         
			0                             
		}
	);

	return entity;
}

  // Entity createGrapplePoint(b2WorldId worldId){
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

	// Outline
	Entity entity_grapple_outline = Entity();
	auto& grapple_outline_motion = registry.motions.emplace(entity_grapple_outline);
	grapple_outline_motion.position = vec2(position.x, position.y);
	grapple_outline_motion.scale = vec2(GRAPPLE_ATTACH_ZONE_RADIUS * 2, GRAPPLE_ATTACH_ZONE_RADIUS * 2);

	// TODO davis fix artificially large attachment zones later LOLOL
	//grapple_outline_motion.scale = vec2(GRAPPLE_ATTACH_ZONE_RADIUS, GRAPPLE_ATTACH_ZONE_RADIUS);

	registry.renderRequests.insert(
		entity_grapple_outline,
		{
			TEXTURE_ASSET_ID::GRAPPLE_OUTLINE,
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
    // djd.maxLength = GRAPPLE_ATTACHABLE_RADIUS;
    djd.maxLength = GRAPPLE_MAX_LENGTH;
	djd.minLength = GRAPPLE_MIN_LENGTH;

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
			EFFECT_ASSET_ID::LEGACY_EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);
	
	registry.colors.insert(entity, vec3(0.0f, 1.0f, 0.0f)); 
	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! line segments between arbitrary enemies_killed
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
			EFFECT_ASSET_ID::LEGACY_EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	registry.colors.insert(entity, vec3(1.0f, 1.0f, 1.0f));
	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! render the entire level png at once
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createLevelTextureLayer(TEXTURE_ASSET_ID textureId)
{
	Entity entity = Entity();
	LevelLayer& levelLayer = registry.levelLayers.emplace(entity);

	auto& motion = registry.motions.emplace(entity);
	motion.position = vec2(WORLD_WIDTH_PX / 2, WORLD_HEIGHT_PX / 2);
	motion.scale = vec2(WORLD_WIDTH_PX, WORLD_HEIGHT_PX);

	registry.renderRequests.insert(
		entity,
		{
			textureId,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! create the entity representing the background layer
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createBackgroundLayer(TEXTURE_ASSET_ID textureId)
{
	Entity entity = Entity();
	BackgroundLayer& backgroundLayer = registry.backgroundLayers.emplace(entity);

	auto& motion = registry.motions.emplace(entity);
	motion.position = vec2(VIEWPORT_WIDTH_PX / 2.f, VIEWPORT_HEIGHT_PX / 2.f);
	motion.scale = vec2(2560, 2256);

	registry.renderRequests.insert(
		entity,
		{
			textureId,
			EFFECT_ASSET_ID::PARALLAX,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createGoalZone(vec2 bottom_left_pos, vec2 bottom_right_pos) {
	// reserve an entity
	auto entity = Entity();

	std::cout << "creating goal post with bottom left: " << bottom_left_pos.x << ", " << bottom_left_pos.y << " and top right: " << bottom_right_pos.x << ", " << bottom_right_pos.y << std::endl;

	GoalZone& goalZone = registry.goalZones.emplace(entity);
	goalZone.bl_boundary = bottom_left_pos;
	goalZone.tr_boundary = bottom_right_pos;
	goalZone.hasTriggered = false;

	return entity;
}