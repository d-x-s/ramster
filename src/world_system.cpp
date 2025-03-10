// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// Global Variables
bool grappleActive = false;

// create the world
WorldSystem::WorldSystem(b2WorldId worldId) :
	points(0),
	max_towers(MAX_TOWERS_START),
	next_enemy_spawn(0),
	enemy_spawn_rate_ms(ENEMY_SPAWN_RATE_MS),
	worldId(worldId)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	// initialize key states with needed keys.
	for (int i = 0; i < PLAYER_CONTROL_KEYS.size(); i++) {
		keyStates[PLAYER_CONTROL_KEYS[i]] = false;
	}
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds() {

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str());
		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem* renderer_arg) {

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

	// Set all states to default
	restart_game();
	createBall(worldId);

	// TODO DAVIS - Create obstacles here so they don't keep spawning.
	// Like this:
	// handleEnemySpawning(true, OBSTACLE, 1, vec2(750, 200 + 50), vec2(800, 1500));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Updating window title with points (and remaining towers)
	std::stringstream title_ss;
	title_ss << "Ramster | Points: " << points << " | FPS: " << fps;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// FPS counter
	if (fps_update_cooldown_ms <= 0) {
		fps = 1 / (elapsed_ms_since_last_update / 1000);
		fps_update_cooldown_ms = FPS_UPDATE_COOLDOWN_MS;
	}
	else {
		fps_update_cooldown_ms -= elapsed_ms_since_last_update;
	}
	

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	if (game_active) {
		update_isGrounded();
		handle_movement();
		checkGrappleGrounded();

		// Removing out of screen entities
		auto& motions_registry = registry.motions;

		/* Given that stuff bounce off map walls we will not need this.. 
		// {{{ OK }}} ??? this is outdated code --> change to remove entities that leave on both the LEFT or RIGHT side
		// Remove entities that leave the screen on the left side
		// Iterate backwards to be able to remove without interfering with the next object to visit
		// (the containers exchange the last element with the current)
		for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
			Motion& motion = motions_registry.components[i];

			float right_edge = motion.position.x + abs(motion.scale.x);  // Rightmost x-coordinate
			float left_edge = motion.position.x - motion.scale.x * 0.5f; // Leftmost x-coordinate

			if (right_edge < 0.f || left_edge > WINDOW_WIDTH_PX) {
				if (!registry.players.has(motions_registry.entities[i])) { // don't remove the player (outdated?)
					// if it is an invader that has exited the screen, trigger game over
					if (registry.invaders.has(motions_registry.entities[i])) { stop_game(); };
					registry.remove_all_components_of(motions_registry.entities[i]);
				}
			}
		}
		*/

		// Spawns new enemies. borrows code from invader spawning.
		next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
		if (next_enemy_spawn <= 0.f) {

			// reset timer
			next_enemy_spawn = (ENEMY_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_RATE_MS / 2);

			//figure out x and y coordinates
			float max_x = WINDOW_WIDTH_PX * 3.0; //this is also the room width
			float max_y = WINDOW_HEIGHT_PX - 100; // this is also room height, adjust by -100 to account for map border

			// random x and y coordinates on the map to spawn enemy
			float pos_x = uniform_dist(rng) * max_x; 
			float pos_y = max_y;  // just spawn on top of screen for now until terrain defined uniform_dist(rng) * max_y;

			// create enemy at random position
			//setting arbitrary pos_y will allow the enemies to spawn pretty much everywhere. Add 50 so it doesn't spawn on edge.
			// handleEnemySpawning(true, COMMON, 1, vec2(pos_x, pos_y + 50), vec2(-1, -1));
			// handleEnemySpawning(true, SWARM, 5, vec2(pos_x, pos_y + 50), vec2(-1, -1));
		}

	}

	return game_active;
}

void WorldSystem::stop_game() {
	// disable player input (except 'R' for restart), see on_key
	game_active = false;

	// trigger gray fadeout
	registry.screenStates.components[0].darken_screen_factor = 0.5f;
	registry.screenStates.components[0].fadeout = 1.0f;

	// stop background music
	if (Mix_PlayingMusic()) {
		Mix_PauseMusic();
	}

	// freeze all entity motion by setting velocities to zero
	auto& motions_registry = registry.motions;
	for (Motion& motion : motions_registry.components) {
		motion.velocity = { 0.0f, 0.0f };
	}
}

void WorldSystem::generateTestTerrain() {
	if (lines.empty()) {
		std::vector<b2Vec2> testPoints = generateTestPoints();

		// reverse vertices for counter-clockwise winding order
		std::reverse(testPoints.begin(), testPoints.end());

		// render the line segments between points
		int count = testPoints.size();
		for (int i = 0; i < count - 1; ++i) {
			lines.push_back(
				createLine(
					glm::vec2(testPoints[i].x, testPoints[i].y), 
					glm::vec2(testPoints[i + 1].x, testPoints[i + 1].y)));
		}

		b2ChainDef chainDef = b2DefaultChainDef();
		chainDef.count = count;
		chainDef.points = testPoints.data();
		chainDef.isLoop = true;
		chainDef.friction = TERRAIN_DEFAULT_FRICTION;
		chainDef.restitution = TERRAIN_DEFAULT_RESTITUTION;

		b2BodyDef bodyDef = b2DefaultBodyDef();
		b2BodyId _ = b2CreateBody(worldId, &bodyDef);
		b2CreateChain(_, &chainDef);
	}
}

std::vector<b2Vec2> WorldSystem::generateTestPoints() {	
	// hardcoded points that make up a ramp
	return {
		{ 0.0f, 288.0f },
		{ 16.67f, 288.0f },
		{ 33.33f, 288.0f },
		{ 50.0f, 288.0f },
		{ 66.67f, 288.0f },
		{ 83.33f, 288.0f },
		{ 100.0f, 288.0f },
		{ 116.67f, 258.67f },
		{ 133.33f, 229.33f },
		{ 150.0f, 200.0f },
		{ 166.67f, 176.0f },
		{ 183.33f, 152.0f },
		{ 200.0f, 128.0f },
		{ 216.67f, 109.33f },
		{ 233.33f, 90.67f },
		{ 250.0f, 72.0f },
		{ 266.67f, 58.67f },
		{ 283.33f, 45.33f },
		{ 300.0f, 32.0f },
		{ 316.67f, 24.0f },
		{ 333.33f, 16.0f },
		{ 350.0f, 8.0f },
		{ 366.67f, 5.33f },
		{ 383.33f, 2.67f },
		{ 400.0f, 0.0f },
		{ 266.67f, 0.0f },
		{ 133.33f, 0.0f },
		{ 0.0f, 0.0f },
		{ 0.0f, 96.0f },
		{ 0.0f, 192.0f },
		{ 0.0f, 288.0f }
	};
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	max_towers = MAX_TOWERS_START;
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;
	grappleActive = false;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// {{{ OK }}} TODO A1: create grid lines
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int grid_line_width = GRID_LINE_WIDTH_PX;

	// render room lines

	// Room dimensions in Box2D world coordinates
	const float roomWidth = 20.0f;
	const float roomHeight = 15.0f;
	const float halfWidth = roomWidth / 2.0f;

	// create grid lines if they do not already exist
	if (grid_lines.size() == 0) {
		// vertical lines
		int cell_width = GRID_CELL_WIDTH_PX;
		for (int col = 0; col < 42 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, WINDOW_HEIGHT_PX)));
		}

		// horizontal lines
		int cell_height = GRID_CELL_HEIGHT_PX;
		for (int col = 0; col < 10 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(0, col * cell_height), vec2(WINDOW_WIDTH_PX * 3.0, grid_line_width)));
		}
	}

	// generate the vertices for the terrain formed by the chain and render it
	generateTestTerrain();

	//create grapple point
	createGrapplePoint(worldId, vec2(1200.0f, 300.0f));

	createGrapplePoint(worldId, vec2(900.0f, 300.0f));

	createGrapplePoint(worldId, vec2(300.0f, 300.0f));

	// turn off trigger for fadeout shader
	registry.screenStates.components[0].fadeout = 0.0f;

	// turn the tunes back on
	if (Mix_PausedMusic()) {
		Mix_ResumeMusic();
	}
	else {
		Mix_PlayMusic(background_music, -1);
	}

	// reactivate the game
	game_active = true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	// This is mostly a repurposing of collision handling implementation from A1
	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity entity = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity other = collision.other; // the other entity in the collision

		// Player - Enemy Collision
		if ((registry.enemies.has(entity) && registry.players.has(other)) ||
			(registry.enemies.has(other) && registry.players.has(entity))) {

			if (registry.enemies.has(entity)) {

				// Figure out the position, velocity characteristics of player and enemy
				Entity enemyEntity = entity;
				Enemy& enemyComponent = registry.enemies.get(enemyEntity);
				Entity playerEntity = other;
				PhysicsBody& enemyPhys = registry.physicsBodies.get(enemyEntity);
				b2BodyId enemyBodyId = enemyPhys.bodyId;
				PhysicsBody& playerPhys = registry.physicsBodies.get(playerEntity);
				b2BodyId playerBodyId = playerPhys.bodyId;

				// For now we'll base everything entirely on speed.
				// Handling based on whether player comes out on top in this collision
				if (collision.player_wins_collision && enemyComponent.destructable) {
					b2DestroyBody(enemyBodyId);
					registry.remove_all_components_of(enemyEntity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
					points++;
				}
				// Otherwise player takes dmg (just loses pts for now) and we freeze the enemy momentarily. 
				// If the enemy is still frozen, player will not be punished.
				else if (enemyComponent.freeze_time <= 0) {
					enemyComponent.freeze_time = ENEMY_FREEZE_TIME_MS;
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					points -= 3; // small penalty for now
				}

			}
			else { 
				
				// Figure out the position, velocity characteristics of player and enemy
				Entity enemyEntity = other;
				Enemy& enemyComponent = registry.enemies.get(enemyEntity);
				Entity playerEntity = entity;
				PhysicsBody& enemyPhys = registry.physicsBodies.get(enemyEntity);
				b2BodyId enemyBodyId = enemyPhys.bodyId;
				PhysicsBody& playerPhys = registry.physicsBodies.get(playerEntity);
				b2BodyId playerBodyId = playerPhys.bodyId;

				// Handling based on whether player comes out on top in this collision
				if (collision.player_wins_collision && enemyComponent.destructable) {
					b2DestroyBody(enemyBodyId);
					registry.remove_all_components_of(other);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
					points++;
				}
				// Otherwise player takes dmg (just loses pts for now) and we freeze the enemy momentarily. 
				// If the enemy is still frozen, player will not be punished.
				else if (enemyComponent.freeze_time <= 0) {
					enemyComponent.freeze_time = ENEMY_FREEZE_TIME_MS;
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					points -= 3; // small penalty for now
				}

			}

			// LEGACY CODE (Pre-Box2D Collision Handling)
			/*
			b2Vec2 enemyPosition = b2Body_GetPosition(enemyBodyId);
			b2Vec2 enemyVelocity = b2Body_GetLinearVelocity(enemyBodyId);
			float enemySpeed = sqrt((enemyVelocity.x * enemyVelocity.x) + (enemyVelocity.y * enemyVelocity.y)); //pythagorean to get speed from velocity
			b2Vec2 playerPosition = b2Body_GetPosition(playerBodyId);
			b2Vec2 playerVelocity = b2Body_GetLinearVelocity(playerBodyId);
			float playerSpeed = sqrt((playerVelocity.x * playerVelocity.x) + (playerVelocity.y * playerVelocity.y)); //pythagorean to get speed from velocity
			*/
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

void WorldSystem::update_isGrounded() {
	Entity playerEntity = registry.players.entities[0];
	PhysicsBody& phys = registry.physicsBodies.get(playerEntity);
	b2BodyId bodyId = phys.bodyId;

	// calculate if ball is grounded or not.
	int num_contacts = b2Body_GetContactCapacity(bodyId);
	bool& isGroundedRef = registry.playerPhysics.get(playerEntity).isGrounded;

	if (num_contacts == 0) {
		isGroundedRef = false;
		return;
	}


	b2ContactData * contactData = new b2ContactData[num_contacts];
	b2Body_GetContactData(bodyId, contactData, num_contacts);

	// get the ball's shape id.
	// The # shapes should always be 1, since the player is initialized as a singular ball shape!
	int player_num_shapes = b2Body_GetShapeCount(bodyId);
	b2ShapeId* shapeArray = new b2ShapeId[player_num_shapes];
	b2Body_GetShapes(bodyId, shapeArray, player_num_shapes);

	b2ShapeId player_shape = shapeArray[0];

	for (int i = 0; i < num_contacts; i++) {
		b2ContactData contact = contactData[i];
		
		// if the collision involves the player.
		if ((contact.shapeIdA.index1 == player_shape.index1 || contact.shapeIdB.index1 == player_shape.index1)) {
			b2Manifold manifold = contact.manifold;
			b2Vec2 normal = manifold.normal;

      if (normal.y >= BALL_ISGROUNDED_NORMAL_THRESHOLD)
      {
        isGroundedRef = true;
        delete[] contactData;
        return;
      }
    }
  }

	isGroundedRef = false;
	delete[] contactData;
}

// call inside step() function for the most precise and responsive movement handling.
void WorldSystem::handle_movement() {

	// first, update states.
	int state = glfwGetKey(window, GLFW_KEY_E);

	for (int i = 0; i < PLAYER_CONTROL_KEYS.size(); i++) {
		int key = PLAYER_CONTROL_KEYS[i];
		int action = glfwGetKey(window, key);

		// set the keyState
		if (action == GLFW_PRESS) {
			keyStates[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			keyStates[key] = false;
		}

	}

	b2Vec2 nonjump_movement_force = { 0, 0 };
	b2Vec2 jump_impulse   = { 0, 0 };
	const float forceMagnitude = BALL_GROUNDED_MOVEMENT_FORCE;
	const float jumpImpulseMagnitude = BALL_JUMP_IMPULSE;

	// Determine impulse direction based on key pressed
	if (keyStates[GLFW_KEY_W]) {
		// nonjump_movement_force = { 0, forceMagnitude };
		if (grappleActive) {
			for (Entity grappleEntity : registry.grapples.entities) {
        		Grapple& grapple = registry.grapples.get(grappleEntity);
				float curLen = b2DistanceJoint_GetCurrentLength(grapple.jointId);
				if (curLen >= 50.0f) {
					b2DistanceJoint_SetLength(grapple.jointId , curLen - 5.0f);
				}
			}
		}
	}
	else if (keyStates[GLFW_KEY_A]) {
		if (grappleActive) {
			nonjump_movement_force = { -forceMagnitude * GRAPPLE_STRAFE_SPEED_MULTIPLIER, 0 };
		} else {
			nonjump_movement_force = { -forceMagnitude, 0 };
		}
	}
	else if (keyStates[GLFW_KEY_S]) {
		// nonjump_movement_force = { 0, -forceMagnitude };
	}
	else if (keyStates[GLFW_KEY_D]) {
		if (grappleActive) {
			nonjump_movement_force = { forceMagnitude * GRAPPLE_STRAFE_SPEED_MULTIPLIER, 0 };
		} else {
			nonjump_movement_force = { forceMagnitude, 0 };
		}
	}

	// jump is set seperately, since it can be used in conjunction with the movement keys.
	if (keyStates[GLFW_KEY_SPACE]) {
		// Jump: apply a strong upward impulse
		jump_impulse = { 0, jumpImpulseMagnitude };
	}

	// Apply impulse if non-zero.
	if (nonjump_movement_force != b2Vec2_zero || jump_impulse != b2Vec2_zero) {
		// Assuming registry.players.entities[0] holds the player entity.
		if (!registry.players.entities.empty()) {
			Entity playerEntity = registry.players.entities[0];

			if (registry.physicsBodies.has(playerEntity)) {
				PhysicsBody& phys = registry.physicsBodies.get(playerEntity);
				b2BodyId bodyId = phys.bodyId;

				// make sure player is grounded.
				bool isGrounded = registry.playerPhysics.get(playerEntity).isGrounded;

				// if jump is registered, it should override any other force being applied.
				if (jump_impulse != b2Vec2_zero && isGrounded) {
					b2Body_ApplyLinearImpulseToCenter(bodyId, jump_impulse, true);
				}
				else if (nonjump_movement_force != b2Vec2_zero) {
					float multiplier = 1.0f;

					if (!isGrounded) {
						multiplier = BALL_AIR_STRAFE_FORCE_MULTIPLIER;
					}

					// apply force slightly above center of mass to make ball spin.
					// don't get the reference of the position, we don't want to alter the value.
					b2Vec2 bodyPosition = b2Body_GetPosition(bodyId);
					bodyPosition.y += 2.f;
					b2Body_ApplyForce(bodyId, nonjump_movement_force * multiplier, bodyPosition, true);
				}


				// apply drag seperately from all other movement forces:
				// drag is there to bring the ball's speed down to the soft cap to prevent them from moving at top speeds all the time.
				b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
				float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
				std::cout << "Ball speed: " << speed << std::endl;
				if (speed > BALL_NONGRAPPLE_SOFT_SPEED_CAP) {
					std::cout << "Applying drag" << std::endl;

					float overspeed = speed - BALL_NONGRAPPLE_SOFT_SPEED_CAP;
					// DRAG_COEFFICIENT is a constant you define to tune the deceleration rate.
					float dragMagnitude = BALL_DRAG_COEFFICIENT * overspeed;
					b2Vec2 dragForce = -velocity;
					dragForce = b2Normalize(dragForce);
					dragForce *= dragMagnitude;
					b2Body_ApplyForce(bodyId, dragForce, b2Body_GetPosition(bodyId), true);
				}

				// update ramster sprite to flamming if reaching top speeds.
				// M3 TODO: We should render the flame effect as seperate entity below the ball in the future.
				RenderRequest& ballRR = registry.renderRequests.get(playerEntity);
				if (speed > RAMSTER_FLAME_THRESHOLD) {
					ballRR.used_texture = TEXTURE_ASSET_ID::RAMSTER_FLAMMING;
				}
				else {
					ballRR.used_texture = TEXTURE_ASSET_ID::RAMSTER_1;
				}
			}
		}
	}

}

// on key callback
void WorldSystem::on_key(int key, int scancode, int action, int mod) {

	if (!game_active) {
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			restart_game();
		}
		return; // ignore all other inputs when game is inactive
	}

	// Exit game with ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		close_window();
	}

	// Reset game when R is released
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		restart_game();
	}

	// Debug toggle with D
	if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
		debugging.in_debug_mode = !debugging.in_debug_mode;
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
	//std::cout << "mouse coordinate position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
    if (!game_active) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Get mouse position and convert to world coordinates.
        vec2 mouseScreenPos = { mouse_pos_x, mouse_pos_y };
        vec2 worldMousePos = screenToWorld(mouseScreenPos);
        std::cout << "Mouse clicked at world position: (" 
                  << worldMousePos.x << ", " << worldMousePos.y << ")" << std::endl;

        // Find the grapple point closest to the click that is within the threshold.
        GrapplePoint* selectedGp = nullptr;
        float bestDist = 50.0f;  // distance threshold

        for (Entity gpEntity : registry.grapplePoints.entities) {
            GrapplePoint& gp = registry.grapplePoints.get(gpEntity);
            float dist = length(gp.position - worldMousePos);
            if (dist < bestDist) {
                bestDist = dist;
                selectedGp = &gp;
            }
        }

        // Deactivate all grapple points.
        for (Entity gpEntity : registry.grapplePoints.entities) {
            GrapplePoint& gp = registry.grapplePoints.get(gpEntity);
            gp.active = false;
        }

        // If a valid grapple point is found, mark it as active.
        if (selectedGp != nullptr) {
            selectedGp->active = true;
            std::cout << "Selected grapple point at (" 
                      << selectedGp->position.x << ", " << selectedGp->position.y 
                      << ") with active = " << selectedGp->active << std::endl;
        }

        // Now, if no grapple is currently attached and we found an active point, attach the grapple.
        if (!grappleActive && selectedGp != nullptr) {
            attachGrapple();
        } else if (grappleActive) {
            removeGrapple();
            grappleActive = false;
        }
    }
}


vec2 WorldSystem::screenToWorld(vec2 mouse_position) {
	for (Entity cameraEntity : registry.cameras.entities) {
		Camera& camera = registry.cameras.get(cameraEntity);
		// Calculate the screen center.
		float centerX = WINDOW_WIDTH_PX / 2.0f;
		float centerY = WINDOW_HEIGHT_PX / 2.0f;

		// Flip the Y coordinate: if the screen's Y=0 is at the top,
		// convert it so Y increases upward. This example assumes that
		// your world-space Y increases upward.
		float flippedY = WINDOW_HEIGHT_PX - mouse_position.y;

		// Offset the mouse position relative to the screen center.
		float offsetX = mouse_position.x - centerX;
		float offsetY = flippedY - centerY;

		// Now add the camera's world position.
		// camera.position is the world-space coordinate at the screen center.
		vec2 worldPos;
		worldPos.x = offsetX + camera.position.x;
		worldPos.y = offsetY + camera.position.y;
		std::cout << "mouse coordinate position: " << worldPos.x<< ", " << worldPos.y << std::endl;
		return worldPos;
	}
}

void WorldSystem::attachGrapple() {
    Entity playerEntity = registry.players.entities[0];

    // Retrieve the ball's physics body
    PhysicsBody& ballBody = registry.physicsBodies.get(playerEntity);
    b2BodyId ballBodyId = ballBody.bodyId;
    b2Vec2 ballPos = b2Body_GetPosition(ballBodyId);

    Entity activeGrapplePointEntity;
    b2BodyId activeGrappleBodyId;
    bool foundActive = false;

    // Loop through all grapple points and find the active one
    for (Entity gpEntity : registry.grapplePoints.entities) {
        GrapplePoint& gp = registry.grapplePoints.get(gpEntity);
		std::cout << gp.position.x << " " << gp.position.y <<  " " << gp.active << std::endl;
        if (gp.active) {
            activeGrapplePointEntity = gpEntity;
            activeGrappleBodyId = gp.bodyId;
            foundActive = true;
            break; // Stop once we find the first active grapple point
        }
    }

    // If no active grapple points were found, exit early
    if (!foundActive) {
        return;
    }

    b2Vec2 grapplePos = b2Body_GetPosition(activeGrappleBodyId);

    // Compute the distance between the ball and the grapple point
    float distance = sqrtf((grapplePos.x - ballPos.x) * (grapplePos.x - ballPos.x) +
                           (grapplePos.y - ballPos.y) * (grapplePos.y - ballPos.y));

    // Attach the grapple if within range
    if (distance <= GRAPPLE_RANGE) {
        createGrapple(worldId, ballBodyId, activeGrappleBodyId, distance);
        grappleActive = true;
    }
}

void WorldSystem:: checkGrappleGrounded() {
	if (grappleActive) {
		if (!registry.players.entities.empty()) {
			Entity playerEntity = registry.players.entities[0];

			if (registry.physicsBodies.has(playerEntity)) {
				PhysicsBody& phys = registry.physicsBodies.get(playerEntity);
				b2BodyId bodyId = phys.bodyId;

				// check if the player is grounded
				bool isGrounded = registry.playerPhysics.get(playerEntity).isGrounded;
				Grapple grapple;
				for (Entity grappleEntity : registry.grapples.entities) {
					grapple = registry.grapples.get(grappleEntity);
				}
				float curLen = b2DistanceJoint_GetCurrentLength(grapple.jointId);
				if (isGrounded) {
					b2DistanceJoint_EnableSpring(grapple.jointId, true);
					b2DistanceJoint_SetSpringHertz(grapple.jointId, 1.0f);
					b2DistanceJoint_SetSpringDampingRatio(grapple.jointId, 0.5f);
					if (curLen >= 50.0f) {
						b2DistanceJoint_SetLength(grapple.jointId , curLen - 20.0f);
					}
				} else {
					b2DistanceJoint_EnableSpring(grapple.jointId, false);
				}
			}
		}
	}
}

void WorldSystem::handleEnemySpawning(bool predicate, ENEMY_TYPES enemy_type, int quantity, vec2 position, vec2 movement_area) {
	
	// only create if predicate is true
	if (predicate) {
		// Create specified number of enemies by iterating
		for (int i = 0; i < quantity; i++) {
			// enemy created here
			createEnemy(worldId, position, enemy_type, movement_area);
		}
	}
}

