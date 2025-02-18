// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem(b2WorldId worldId) :
	points(0),
	max_towers(MAX_TOWERS_START),
	next_invader_spawn(0),
	invader_spawn_rate_ms(INVADER_SPAWN_RATE_MS),
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
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Updating window title with points (and remaining towers)
	std::stringstream title_ss;
	title_ss << "Ramster | Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	if (game_active) {
		handle_movement();

		// Removing out of screen entities
		auto& motions_registry = registry.motions;

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

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	max_towers = MAX_TOWERS_START;
	next_invader_spawn = 0;
	invader_spawn_rate_ms = INVADER_SPAWN_RATE_MS;

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
		for (int col = 0; col < 14 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
		}

		// horizontal lines
		int cell_height = GRID_CELL_HEIGHT_PX;
		for (int col = 0; col < 10 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(0, col * cell_height), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
		}
	}

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

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// {{{ OK }}} TODO A1: Loop over all collisions detected by the physics system
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity entity = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity other = collision.other; // the other entity in the collision

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// {{{ OK }}} TODO A1: handle collision between deadly (projectile) and invader
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if ((registry.projectiles.has(entity) && registry.invaders.has(other)) ||
			(registry.projectiles.has(other) && registry.invaders.has(entity))) {
			if (registry.invaders.has(entity)) {
				Invader& invader_component = registry.invaders.get(entity);
				Projectile& projectile_component = registry.projectiles.get(other);
				invader_component.health -= projectile_component.damage;
				if (invader_component.health <= 0) {
					// explosion
					Motion& motion = registry.motions.get(entity);
					createExplosion(renderer, motion.position);
					// remove tower and invader
					registry.remove_all_components_of(entity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
					points += 1;
				}
				registry.remove_all_components_of(other);
			}
			else { // it must be that registry.invaders.has(other)
				Invader& invader_component = registry.invaders.get(other);
				Projectile& projectile_component = registry.projectiles.get(entity);
				invader_component.health -= projectile_component.damage;
				if (invader_component.health <= 0) {
					// explosion
					Motion& motion = registry.motions.get(other);
					createExplosion(renderer, motion.position);
					// remove tower and invader
					registry.remove_all_components_of(other);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
					points += 1;
				}
				registry.remove_all_components_of(entity);
			}
			continue;
		}

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// {{{ OK }}} TODO A1: handle collision between tower and invader
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if ((registry.towers.has(entity) && registry.invaders.has(other)) ||
			(registry.towers.has(other) && registry.invaders.has(entity))) {
			if (registry.invaders.has(entity)) {
				// explosion
				Motion& motion = registry.motions.get(entity);
				createExplosion(renderer, motion.position);
				// remove tower and invader
				Invader& invader_component = registry.invaders.get(entity);
				Tower& tower_component = registry.towers.get(other);
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(other);
			}
			else { // it must be that registry.invaders.has(other)
				// explosion
				Motion& motion = registry.motions.get(other);
				createExplosion(renderer, motion.position);
				// remove tower and invader
				Invader& invader_component = registry.invaders.get(other);
				Tower& tower_component = registry.towers.get(entity);
				registry.remove_all_components_of(other);
				registry.remove_all_components_of(entity);
			}

			// activate vignette effect
			registry.screenStates.components[0].darken_screen_factor = 1.0f;
			registry.screenStates.components[0].vignette = 1.0f;
			trigger_vignette(2000.0f);

			Mix_PlayChannel(-1, chicken_eat_sound, 0);
			max_towers -= 1;
			continue;
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
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
	const float forceMagnitude = GROUNDED_MOVEMENT_FORCE_SPEED;
	const float jumpImpulseMagnitude = JUMP_IMPULSE_SPEED;

	// Determine impulse direction based on key pressed
	if (keyStates[GLFW_KEY_W]) {
		nonjump_movement_force = { 0, forceMagnitude };
	}
	else if (keyStates[GLFW_KEY_A]) {
		nonjump_movement_force = { -forceMagnitude, 0 };
	}
	else if (keyStates[GLFW_KEY_S]) {
		nonjump_movement_force = { 0, -forceMagnitude };
	}
	else if (keyStates[GLFW_KEY_D]) {
		nonjump_movement_force = { forceMagnitude, 0 };
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


				// if jump is registered, it should override any other force being applied.
				if (jump_impulse != b2Vec2_zero) {
					b2Body_ApplyLinearImpulseToCenter(bodyId, jump_impulse, true);
				}
				else if (nonjump_movement_force != b2Vec2_zero) {
					// apply force slightly above center of mass to make ball spin.
					// don't get the reference of the position, we don't want to alter the value.
					std::cout << "Applying movement force on ball. ================================================" << std::endl;
					b2Vec2 bodyPosition = b2Body_GetPosition(bodyId);
					bodyPosition.y += 3.f;
					b2Body_ApplyForce(bodyId, nonjump_movement_force, bodyPosition, true);
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
	std::cout << "mouse coordinate position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// {{{ OK }}} TODO A1: Handle mouse clicking for invader and tower placement.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// ignore mouse input if game is over
	if (!game_active) {
		return;
	}

	// on button press
	if (action == GLFW_PRESS) {

		int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
		int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

		std::cout << "mouse button: " << button << std::endl;
		std::cout << "mouse action: " << action << std::endl;
		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// {{{ OK }}} TODO A1: place invaders on the left, except top left spot
		// - A mouse left-click on the far-left side of the screen (column 0) will result in
		//   an invader spawning in the current cell(except for cell 0, 0)
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (tile_x == 0 && tile_y != 0) {
			float invader_x_pos = (tile_x + 0.5f) * GRID_CELL_WIDTH_PX;
			float invader_y_pos = (tile_y + 0.5f) * GRID_CELL_HEIGHT_PX;
			createInvader(renderer, vec2(invader_x_pos, invader_y_pos));
			return;
		}

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// {{{ OK }}} TODO A1: place a tower on the right, except top right spot
		// - A mouse left-click on the farright side of the screen (column 13) 
		//   will result in a tower being spawned in that cell.
		// - If an existing tower is present, it will remove and replace with a new tower
		// - A mouse right-click on the farright of the screen will remove the tower in that cell
		//   if it exists there
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (tile_x == 13 && tile_y != 0) {
			vec2 tower_pos = vec2((tile_x + 0.5f) * GRID_CELL_WIDTH_PX,
				(tile_y + 0.5f) * GRID_CELL_HEIGHT_PX);
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// {{{ OK }}} TODO A1: right-click removes towers
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				for (Entity& tower_entity : registry.towers.entities) {
					const Motion& tower_motion = registry.motions.get(tower_entity);
					if (tower_motion.position == tower_pos) {
						removeTower(tower_pos);
						return;
					}
				}
			}
			else if (button == GLFW_MOUSE_BUTTON_LEFT) {
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// {{{ OK }}} TODO A1: left-click adds new tower (removing any existing towers), up to max_towers
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				for (Entity& tower_entity : registry.towers.entities) {
					const Motion& tower_motion = registry.motions.get(tower_entity);
					if (tower_motion.position == tower_pos) {
						removeTower(tower_pos);
						createTower(renderer, tower_pos);
						return;
					}
				}
				int numTowers = registry.towers.entities.size();
				if (numTowers < max_towers) {
					createTower(renderer, tower_pos);
					return;
				}
				else {
					std::cout << "cannot place any more towers, max towers is: " << max_towers <<
						" and there are currently this number of towers: " << numTowers << std::endl;
					return;
				}
			}
		}
	}
}