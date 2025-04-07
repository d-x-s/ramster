// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <tuple>
#include <vector>
#include <json/json.h>
#include <box2d/box2d.h>

// internal
#include "physics_system.hpp"
#include "terrain.hpp"

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);

  WorldSystem *world = (WorldSystem *)glfwGetWindowUserPointer(window);
  if (world && world->renderer)
  {
    world->renderer->resizeScreenTexture(width, height);
  }
}

// Global Variables
bool grappleActive = false;
// No longer need this - maps onto current level MUSIC current_music = MUSIC::LEVEL_1;

// create the world
WorldSystem::WorldSystem(b2WorldId worldId) : enemies_killed(0),
                                              max_towers(MAX_TOWERS_START),
                                              next_enemy_spawn(0),
                                              enemy_spawn_rate_ms(ENEMY_SPAWN_RATE_MS),
                                              worldId(worldId)
{
  // seeding rng with random device
  rng = std::default_random_engine(std::random_device()());

  // initialize key states with needed keys.
  for (int i = 0; i < PLAYER_CONTROL_KEYS.size(); i++)
  {
    keyStates[PLAYER_CONTROL_KEYS[i]] = false;
  }
}

WorldSystem::~WorldSystem()
{
  // Destroy music components
  if (background_music != nullptr)
    Mix_FreeMusic(background_music);
  if (background_music_memorybranch != nullptr)
    Mix_FreeMusic(background_music_memorybranch);
  if (background_music_oblanka != nullptr)
    Mix_FreeMusic(background_music_oblanka);
  if (background_music_paradrizzle != nullptr)
    Mix_FreeMusic(background_music_paradrizzle);
  if (background_music_windcatcher != nullptr)
    Mix_FreeMusic(background_music_windcatcher);
  if (background_music_promenade != nullptr)
    Mix_FreeMusic(background_music_promenade);
  if (background_music_spaba != nullptr)
    Mix_FreeMusic(background_music_spaba);
  if (background_music_cottonplanes != nullptr)
    Mix_FreeMusic(background_music_cottonplanes);
  if (background_music_pencilcrayons != nullptr)
    Mix_FreeMusic(background_music_pencilcrayons);
  if (background_music_moontownshores != nullptr)
    Mix_FreeMusic(background_music_moontownshores);
  if (fx_destroy_enemy != nullptr)
    Mix_FreeChunk(fx_destroy_enemy);
  if (fx_destroy_enemy_fail != nullptr)
    Mix_FreeChunk(fx_destroy_enemy_fail);
  if (fx_jump != nullptr)
    Mix_FreeChunk(fx_jump);
  if (fx_grapple != nullptr)
    Mix_FreeChunk(fx_grapple);
  if (fx_victory != nullptr)
    Mix_FreeChunk(fx_victory);
  if (chicken_dead_sound != nullptr)
    Mix_FreeChunk(chicken_dead_sound);
  if (chicken_eat_sound != nullptr)
    Mix_FreeChunk(chicken_eat_sound);
  if (ball_rolling != nullptr)
    Mix_FreeChunk(ball_rolling);
  if (ball_flamming != nullptr)
    Mix_FreeChunk(ball_flamming);
  Mix_CloseAudio();

  // Destroy all created components
  registry.clear_all_components();

  // Close the window
  glfwDestroyWindow(window);
}

// Debugging
namespace
{
  void glfw_err_cb(int error, const char *desc)
  {
    std::cerr << error << ": " << desc << std::endl;
  }
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window()
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{

  ///////////////////////////////////////
  // Initialize GLFW
  glfwSetErrorCallback(glfw_err_cb);
  if (!glfwInit())
  {
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
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  // CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE); // GLFW 3.3+
  // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE); // GLFW 3.3+

  // Create the main window (for rendering, keyboard, and mouse input)
  window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Ramster", nullptr, nullptr);
  if (window == nullptr)
  {
    std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
    return nullptr;
  }

  // Fill the whole monitor
  glfwMaximizeWindow(window);

  // Setting callbacks to member functions (that's why the redirect is needed)
  // Input is handled using GLFW, for more info see
  // http://www.glfw.org/docs/latest/input_guide.html
  glfwSetWindowUserPointer(window, this);
  auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
  { ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
  auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
  { ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
  auto mouse_button_pressed_redirect = [](GLFWwindow *wnd, int _button, int _action, int _mods)
  { ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);
  glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

  return window;
}

bool WorldSystem::start_and_load_sounds()
{

  //////////////////////////////////////
  // Loading music and sounds with SDL
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
    fprintf(stderr, "Failed to initialize SDL Audio");
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
  {
    fprintf(stderr, "Failed to open audio device");
    return false;
  }

  // music
  background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
  background_music_story_intro = background_music;
  background_music_story_conclusion = background_music;
  background_music_memorybranch = Mix_LoadMUS(audio_path("music_memorybranch.wav").c_str());
  background_music_oblanka = Mix_LoadMUS(audio_path("music_oblanka.wav").c_str());
  background_music_paradrizzle = Mix_LoadMUS(audio_path("music_paradrizzle.wav").c_str());
  background_music_windcatcher = Mix_LoadMUS(audio_path("music_windcatcher.wav").c_str());
  background_music_promenade = Mix_LoadMUS(audio_path("music_promenade.wav").c_str());
  background_music_spaba = Mix_LoadMUS(audio_path("music_spaba.wav").c_str());
  background_music_cottonplanes = Mix_LoadMUS(audio_path("music_cottonplanes.wav").c_str());
  background_music_pencilcrayons = Mix_LoadMUS(audio_path("music_pencilcrayons.wav").c_str());
  background_music_moontownshores = Mix_LoadMUS(audio_path("music_moontownshores.wav").c_str());

  // sound fx
  fx_destroy_enemy = Mix_LoadWAV(audio_path("fx_destroy_enemy.wav").c_str());
  fx_destroy_enemy_fail = Mix_LoadWAV(audio_path("fx_destroy_enemy_fail.wav").c_str());
  fx_jump = Mix_LoadWAV(audio_path("fx_jump.wav").c_str());
  fx_grapple = Mix_LoadWAV(audio_path("fx_grapple.wav").c_str());
  fx_victory = Mix_LoadWAV(audio_path("fx_victory.wav").c_str());
  chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
  chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
  ball_rolling = Mix_LoadWAV(audio_path("ball_rolling_sfx.wav").c_str());
  ball_flamming = Mix_LoadWAV(audio_path("fire_woosh_sfx.wav").c_str());
  ramster_scream = Mix_LoadWAV(audio_path("ramster_scream.wav").c_str());
  im_going_ham = Mix_LoadWAV(audio_path("im_going_ham.wav").c_str());
  
  if (
      background_music == nullptr ||
      background_music_memorybranch == nullptr ||
      background_music_oblanka == nullptr ||
      background_music_paradrizzle == nullptr ||
      background_music_windcatcher == nullptr ||
	  background_music_promenade == nullptr ||
	  background_music_spaba == nullptr ||
	  background_music_cottonplanes == nullptr ||
	  background_music_pencilcrayons == nullptr ||
	  background_music_moontownshores == nullptr ||

	  ball_rolling == nullptr ||
	  ball_flamming == nullptr ||
	  ramster_scream == nullptr ||
	  im_going_ham == nullptr ||

      fx_destroy_enemy == nullptr ||
      fx_destroy_enemy_fail == nullptr ||
      fx_jump == nullptr ||
      fx_grapple == nullptr ||
      chicken_dead_sound == nullptr ||
      chicken_eat_sound == nullptr)
  {
    fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
            // music
            audio_path("music.wav").c_str(),
            audio_path("music_memorybranch.wav").c_str(),
            audio_path("music_oblanka.wav").c_str(),
            audio_path("music_paradrizzle.wav").c_str(),
            audio_path("music_windcatcher.wav").c_str(),
            audio_path("music_promenade.wav").c_str(),
            audio_path("music_spaba.wav").c_str(),
            audio_path("music_cottonplanes.wav").c_str(),
            audio_path("music_pencilcrayons.wav").c_str(),
            audio_path("music_moontownshores.wav").c_str(),

            // sound fx
            audio_path("fx_destroy_enemy.wav").c_str(),
            audio_path("fx_destroy_enemy_fail.wav").c_str(),
            audio_path("fx_jump.wav").c_str(),
            audio_path("fx_grapple.wav").c_str(),
            audio_path("chicken_dead.wav").c_str(),
            audio_path("chicken_eat.wav").c_str());
    return false;
  }

  return true;
}

void WorldSystem::playMusic(MUSIC music)
{
  switch (music)
  {
  case MUSIC::MENU:
    Mix_PlayMusic(background_music_memorybranch, -1);
    break;
  case MUSIC::OBLANKA:
    Mix_PlayMusic(background_music_oblanka, -1);
    break;
  case MUSIC::PARADRIZZLE:
    Mix_PlayMusic(background_music_paradrizzle, -1);
    break;
  case MUSIC::WINDCATCHER:
    Mix_PlayMusic(background_music_windcatcher, -1);
    break;
  case MUSIC::PROMENADE:
    Mix_PlayMusic(background_music_promenade, -1);
    break;
  case MUSIC::SPABA:
    Mix_PlayMusic(background_music_spaba, -1);
    break;
  case MUSIC::COTTONPLANES:
    Mix_PlayMusic(background_music_cottonplanes, -1);
    break;
  case MUSIC::PENCILCRAYONS:
    Mix_PlayMusic(background_music_pencilcrayons, -1);
    break;
  case MUSIC::MOONTOWNSHORES:
    Mix_PlayMusic(background_music_moontownshores, -1);
    break;
  default:
    Mix_PlayMusic(background_music_memorybranch, -1);
    break;
  }

  Mix_VolumeMusic(4);
}

void WorldSystem::playSoundEffect(FX effect)
{
  int channel;
  switch (effect)
  {
    case FX::FX_DESTROY_ENEMY:
      channel = Mix_PlayChannel(-1, fx_destroy_enemy, 0);
      break;
    case FX::FX_DESTROY_ENEMY_FAIL:
        channel = Mix_PlayChannel(-1, fx_destroy_enemy_fail, 0);
      break;
    case FX::FX_JUMP:
        channel = Mix_PlayChannel(-1, fx_jump, 0);
      break;
    case FX::FX_GRAPPLE:
        channel = Mix_PlayChannel(-1, fx_grapple, 0);
      break;
    default:
        channel = Mix_PlayChannel(-1, fx_destroy_enemy, 0);
      break;
  } 

  Mix_Volume(channel, 5);
}


// should only be called after ramster has defeated an enemy.
void WorldSystem::handleRamsterVoicelines() {
    Entity playerEntity = registry.players.entities[0];
    Player& player = registry.players.get(playerEntity);

    // Get the current time
    auto now = std::chrono::steady_clock::now();

    // Check if the cooldown period has passed
    if (std::chrono::duration_cast<std::chrono::seconds>(now - player.lastVoicelineTime).count() < 2) {
        return; // Cooldown period has not passed, do nothing
    }

    // Random number generator for probability increase
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(5, 15);

    // Increment the probability by a random percentage between 5 and 15
    player.voicelineProbability += dis(gen);

    // Generate a random number between 0 and 100
    std::uniform_int_distribution<> chance(0, 100);
    int randomChance = chance(gen);

    // Check if the random chance is less than or equal to the current probability
    if (randomChance <= player.voicelineProbability) {
        // Play voiceline and reset the probability
        int channel;
        int random = rand() % 2;

        if (random == 0) {
            channel = Mix_PlayChannel(-1, ramster_scream, 0);
        }
        else {
            channel = Mix_PlayChannel(-1, im_going_ham, 0);
        }

        Mix_Volume(channel, 5);

        // Reset the probability
        player.voicelineProbability = 0;

        // Update the last voiceline time
        player.lastVoicelineTime = now;
    }
}

// NOTE: function should be called after update_isGrounded() call for accuracy.
void WorldSystem::handleRollingSfx()
{
  Entity playerEntity = registry.players.entities[0];
  Player &player = registry.players.get(playerEntity);
  PhysicsBody &phys = registry.physicsBodies.get(playerEntity);
  b2BodyId bodyId = phys.bodyId;

  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  bool &isGroundedRef = registry.playerPhysics.get(playerEntity).isGrounded;
  b2Vec2 playerVelocity = b2Body_GetLinearVelocity(bodyId);

  if (isGroundedRef && b2Length(playerVelocity) >= 70 && game_active && currentScreen.current_screen == "PLAYING")
  {
    // start audio on loop.
    if (!player.isCurrentlyRolling)
    {
      player.isCurrentlyRolling = true;
      Mix_HaltChannel(7);
      Mix_FadeInChannelTimed(7, ball_rolling, -1, 600, -1);
      Mix_Volume(7, 50);
    }
  }
  else
  {
    // stop audio and update isCurrentlyRolling
    player.isCurrentlyRolling = false;
    Mix_FadeOutChannel(7, 450);
  }
}

// NOTE: function should be called after update_isGrounded() call for accuracy.
void WorldSystem::handleFlammingSfx()
{
  Entity playerEntity = registry.players.entities[0];
  Player &player = registry.players.get(playerEntity);
  PhysicsBody &phys = registry.physicsBodies.get(playerEntity);
  b2BodyId bodyId = phys.bodyId;

  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  b2Vec2 playerVelocity = b2Body_GetLinearVelocity(bodyId);

  if (b2Length(playerVelocity) >= MIN_COLLISION_SPEED && game_active && currentScreen.current_screen == "PLAYING")
  {
    // start audio on loop.
    if (!player.isCurrentlyFlamming)
    {
      player.isCurrentlyFlamming = true;
      Mix_HaltChannel(6);
      Mix_FadeInChannelTimed(6, ball_flamming, -1, 100, -1);
      Mix_Volume(6, 75);
    }
  }
  else
  {
    // stop audio and update isCurrentlyRolling
    player.isCurrentlyFlamming = false;
    Mix_FadeOutChannel(6, 200);
  }
}

void WorldSystem::init(RenderSystem *renderer_arg)
{

  this->renderer = renderer_arg;

  // start playing background music indefinitely
  // std::cout << "Starting music..." << std::endl;
  // Mix_PlayMusic(background_music, -1);

  // Set all states to default
  restart_game(current_level);
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{

  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  // Updating window title with enemies_killed (and remaining towers)
  std::stringstream title_ss;
  title_ss << "Ramster | Level : " << current_level << " | Time : " << time_elapsed << "s | Kills : " << enemies_killed << " | HP : " << hp << " | FPS : " << fps;
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Game logic only runs when playing
  if (currentScreen.current_screen == "PLAYING")
  {
    // FPS counter
    if (fps_update_cooldown_ms <= 0)
    {
      fps = 1 / (elapsed_ms_since_last_update / 1000);
      fps_update_cooldown_ms = FPS_UPDATE_COOLDOWN_MS;
    }
    else
    {
      fps_update_cooldown_ms -= elapsed_ms_since_last_update;
    }
    // Time elapsed
    if (time_granularity <= 0)
    {
      time_elapsed += 1; // note: this only works if granularity is 1000 ms and time is in seconds
      time_granularity = TIME_GRANULARITY;
    }
    else
    {
      time_granularity -= elapsed_ms_since_last_update;
    }

    if (is_in_goal())
    {
      player_reached_finish_line = true;
    }

    // Remove debug info from the last step
    while (registry.debugComponents.entities.size() > 0)
      registry.remove_all_components_of(registry.debugComponents.entities.back());

    if (game_active)
    {
      handleGameover(currentScreen);
      update_isGrounded();
      handle_movement();
      checkGrappleGrounded();
      handleRollingSfx();
      handleFlammingSfx();
    }

    // This handles the enemy spawning. Refer copied comment from spawn map in header file:
    // Updated map:
    //	key is a vector<int> (tile that triggers spawn),
    //	value is a tuple with:
    // 1. ENEMY_TYPE denoting type of enemy to spawn
    // 2. Int denoting quantity of enemies to spawn
    // 3. Boolean denoting spawnMap
    // 4. Boolean denoting hasEnemyAlreadySpawned (at this tile)
    // 5. vector<int> denoting spawn position
    // 6. vector<int> denoting patrol range on the X-axis
    for (auto &i : spawnMap)
    {
      std::vector<int> spawnTile = i.first;
      std::tuple<ENEMY_TYPES, int, bool, bool, std::vector<int>, std::vector<int>> &enemyDataTuple = i.second;
      ENEMY_TYPES enemyType = std::get<0>(enemyDataTuple);
      int quantity = std::get<1>(enemyDataTuple);
      bool &hasPlayerReachedTile = std::get<2>(enemyDataTuple);
      bool &hasEnemyAlreadySpawned = std::get<3>(enemyDataTuple);
      std::vector<int> spawnPosition = std::get<4>(enemyDataTuple);
      std::vector<int> patrolRange = std::get<5>(enemyDataTuple);

      if (!hasPlayerReachedTile)
      {
        hasPlayerReachedTile = checkPlayerReachedArea(ivec2(spawnTile[0], spawnTile[1]), ivec2(spawnTile[2], spawnTile[3]));
      }

      if (hasPlayerReachedTile && !hasEnemyAlreadySpawned)
      {
        hasEnemyAlreadySpawned = true;
        handleEnemySpawning(
            enemyType,
            quantity,
            ivec2(spawnPosition[0], spawnPosition[1]),
            ivec2(patrolRange[0], patrolRange[1]),
            ivec2(patrolRange[2], patrolRange[3]));
      }
    }
  }

  return game_active;
}

void WorldSystem::stop_game()
{
  // disable player input (except 'R' for restart), see on_key
  game_active = false;

  // trigger gray fadeout
  registry.screenStates.components[0].darken_screen_factor = 0.5f;
  registry.screenStates.components[0].fadeout = 1.0f;

  // stop background music
  if (Mix_PlayingMusic())
  {
    Mix_PauseMusic();
  }

  // freeze all entity motion by setting velocities to zero
  auto &motions_registry = registry.motions;
  for (Motion &motion : motions_registry.components)
  {
    motion.velocity = {0.0f, 0.0f};
  }
}

bool WorldSystem::is_in_goal()
{
  if (registry.goalZones.entities.size() >= 1)
  {
    GoalZone &goalZone = registry.goalZones.get(registry.goalZones.entities[0]);
    Entity &playerEntity = registry.players.entities[0];

    vec2 player_position = registry.motions.get(playerEntity).position;

    vec2 bl = goalZone.bl_boundary;
    vec2 tr = goalZone.tr_boundary;

    // check if player is within goal zone
    if (player_position.x >= bl.x && player_position.x <= tr.x && player_position.y >= bl.y && player_position.y <= tr.y)
    {
      if (!goalZone.hasTriggered)
      {
        //         std::cout << "=====================================================<VICTORY!>=====================================================" << std::endl;
        // std::cout << "player location: " << player_position.x << ", " << player_position.y << std::endl;
        // std::cout << "goal zone location (bl): " << bl.x << ", " << bl.y << std::endl;
        // std::cout << "goal zone location (tr): " << tr.x << ", " << tr.y << std::endl;
        // std::cout << "number of goal posts: " << registry.goalZones.entities.size() << std::endl;

        int channel = Mix_PlayChannel(-1, fx_victory, 0);
        Mix_Volume(channel, 4);
        createConfetti(vec2((bl.x + tr.x) / 2, bl.y + 60.f));
      }

      goalZone.hasTriggered = true;

      return true;
    }
  }
  else
  {
    return false;
  }
}

bool WorldSystem::load_level(const std::string &filename)
{
  const std::string full_filepath = LEVEL_DIR_FILEPATH + filename;
  Json::Value mapData;

  std::ifstream infile(full_filepath);

  if (infile.fail())
  {
    return false;
  }

  infile >> mapData;

  // checks to ensure JSON data is the expected format.
  assert(mapData.find("height") != nullptr);
  assert(mapData.find("width") != nullptr);
  assert(mapData.find("layers") != nullptr);
  assert(mapData["layers"].size() >= 2);
  assert(mapData["layers"][1]["name"] == "Chain"); // this assert fails if map doesn't have at least 1 chainShape.
  assert(mapData["layers"][1].find("objects") != nullptr);

  // set stage dimensions
  WORLD_WIDTH_TILES = mapData["width"].asFloat() / 2;
  WORLD_HEIGHT_TILES = mapData["height"].asFloat() / 2;

  WORLD_WIDTH_PX = WORLD_WIDTH_TILES * GRID_CELL_WIDTH_PX;
  WORLD_HEIGHT_PX = WORLD_HEIGHT_TILES * GRID_CELL_HEIGHT_PX;

  auto &JsonObjects = mapData["layers"][1]["objects"];
  bool spawnpoint_found = false;
  bool goalzone_found = false;

  // Temporary storage for spawn zones and points
  std::unordered_map<std::string, std::vector<vec2>> spawnZones;
  std::unordered_map<std::string, std::vector<vec2>> spawnPoints;
  std::unordered_map<std::string, std::string> enemyType;
  std::unordered_map<std::string, int> enemyQuantity;

  for (const auto &jsonObj : JsonObjects)
  {
    // polyline case
    if (jsonObj.find(JSON_POLYLINE_ATTR) != nullptr)
    {
      std::vector<vec2> chainPoints;
      const float x_offset = jsonObj["x"].asFloat();
      const float y_offset = jsonObj["y"].asFloat();
      const float rotation = jsonObj["rotation"].asFloat() * (M_PI / 180.f);

      for (auto &point : jsonObj[JSON_POLYLINE_ATTR])
      {
        float x = point["x"].asFloat();
        float y = point["y"].asFloat();

        // rotate points
        if (rotation != 0.f)
        {
          vec2 origin = vec2(0.f, 0.f);
          vec2 rotatedPoint = rotateAroundPoint(vec2(x, y), origin, rotation);
          x = rotatedPoint.x;
          y = rotatedPoint.y;
        }

        chainPoints.push_back(vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset)));
      }

      std::string name = jsonObj["name"].asString();
      if (chainPoints.size() == 2 && name.find("SZ") == 0)
      {
        // Process enemy spawn zone
        std::vector<std::string> parts = split(name, "_");
        if (parts.size() == 2)
        {
          std::string id = parts[1];
          spawnZones[id] = chainPoints;
          std::cout << "Found enemy spawn ZONE with id: " << id << std::endl;
        }
      }
      else if (chainPoints.size() == 2 && name.find("ENEMY") == 0)
      {
        std::cout << "found obstacle enemy spawnpath." << std::endl;
        std::vector<std::string> parts = split(name, "_");
        std::vector<vec2> points;
        std::string id = parts[3];

        const float x_offset = jsonObj["x"].asFloat();
        const float y_offset = jsonObj["y"].asFloat();

        for (auto &point : jsonObj[JSON_POLYLINE_ATTR])
        {
          float x = point["x"].asFloat();
          float y = point["y"].asFloat();
          points.push_back(vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset)));
        }

        spawnPoints[id] = points;
        enemyType[id] = parts[1];
        enemyQuantity[id] = std::stoi(parts[2]);
      }
      else if (chainPoints.size() == 2 && name == "goal")
      {
        std::cout << "found goalpost!" << std::endl;
        goalzone_found = true;

        bool first = true;
        vec2 bl_corner;
        vec2 tr_corner;
        for (auto &point : jsonObj[JSON_POLYLINE_ATTR])
        {
          float x = point["x"].asFloat();
          float y = point["y"].asFloat();

          if (first)
          {
            bl_corner = vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset));
            first = false;
          }
          else
          {
            tr_corner = vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset));
          }
        }

        createGoalZone(bl_corner, tr_corner);
      }
      else if (chainPoints.size() >= 2)
      {
        std::cout << "Creating chainShape with " << chainPoints.size() << " segments." << std::endl;
        if (name == "ledge")
        {
          create_chain(worldId, chainPoints, false, lines);
        }
        else
        {
          create_chain(worldId, chainPoints, true, lines);
        }
      }
    }
    // polygon case: polygon = closed-loop chain
    else if (jsonObj.find(JSON_POLYGON_ATTR) != nullptr)
    {
      std::vector<vec2> chainPoints;
      const float x_offset = jsonObj["x"].asFloat();
      const float y_offset = jsonObj["y"].asFloat();
      const float rotation = jsonObj["rotation"].asFloat() * (M_PI / 180.f);

      for (auto &point : jsonObj[JSON_POLYGON_ATTR])
      {
        float x = point["x"].asFloat();
        float y = point["y"].asFloat();

        // rotate enemies_killed
        if (rotation != 0.f)
        {
          vec2 origin = vec2(0.f, 0.f);
          vec2 rotatedPoint = rotateAroundPoint(vec2(x, y), origin, rotation);
          x = rotatedPoint.x;
          y = rotatedPoint.y;
        }

        chainPoints.push_back(vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset)));
      }

      std::string name = jsonObj["name"].asString();
      if (chainPoints.size() == 2 && name.find("SZ") == 0)
      {
        // Process enemy spawn zone
        std::vector<std::string> parts = split(name, "_");
        if (parts.size() == 2)
        {
          std::string id = parts[1];
          spawnZones[id] = chainPoints;
          std::cout << "Found enemy spawn ZONE with id: " << id << std::endl;
        }
      }
      else if (chainPoints.size() == 2 && name.find("ENEMY") == 0)
      {
        std::cout << "found obstacle enemy spawnpath." << std::endl;
        std::vector<std::string> parts = split(name, "_");
        std::vector<vec2> points;
        std::string id = parts[3];

        const float x_offset = jsonObj["x"].asFloat();
        const float y_offset = jsonObj["y"].asFloat();

        for (auto &point : jsonObj[JSON_POLYLINE_ATTR])
        {
          float x = point["x"].asFloat();
          float y = point["y"].asFloat();
          points.push_back(vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset)));
        }

        spawnPoints[id] = points;
        enemyType[id] = parts[1];
        enemyQuantity[id] = std::stoi(parts[2]);
      }
      else if (chainPoints.size() == 2 && name == "goal")
      {
        std::cout << "found goalpost!" << std::endl;
        goalzone_found = true;

        bool first = true;
        vec2 bl_corner;
        vec2 tr_corner;
        for (auto &point : jsonObj[JSON_POLYLINE_ATTR])
        {
          float x = point["x"].asFloat();
          float y = point["y"].asFloat();

          if (first)
          {
            bl_corner = vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset));
            first = false;
          }
          else
          {
            tr_corner = vec2(x + x_offset, WORLD_HEIGHT_PX - (y + y_offset));
          }
        }

        createGoalZone(bl_corner, tr_corner);
      }
      else if (chainPoints.size() >= 2)
      {
        std::cout << "Creating chainShape with " << chainPoints.size() << " segments." << std::endl;
        if (name == "ledge")
        {
          create_chain(worldId, chainPoints, false, lines);
        }
        else
        {
          create_chain(worldId, chainPoints, true, lines);
        }
      }
    }
    // ball_spawnpoint case
    else if (jsonObj.find("name") != nullptr && jsonObj["name"] == JSON_BALL_SPAWNPOINT)
    {
      const float x = jsonObj["x"].asFloat();
      const float y = jsonObj["y"].asFloat();
      createBall(worldId, vec2(x, WORLD_HEIGHT_PX - y));
      spawnpoint_found = true;
    }

    // enemy spawnpoint case
    else if (jsonObj.find("name") != nullptr && jsonObj["name"].asString().find("ENEMY") == 0)
    {
      std::string name = jsonObj["name"].asString();
      std::vector<std::string> parts = split(name, "_");
      if (parts.size() == 4)
      {
        std::string id = parts[3];
        std::vector<vec2> points;

        float x = jsonObj["x"].asFloat();
        float y = jsonObj["y"].asFloat();
        points.push_back(vec2(x, WORLD_HEIGHT_PX - y));

        std::cout << "Found enemy spawn POINT with id: " << id << std::endl;
        spawnPoints[id] = points;
        enemyType[id] = parts[1];
        enemyQuantity[id] = std::stoi(parts[2]);
      }
    }
    // grapple point case
    else if (jsonObj.find("name") != nullptr && jsonObj["name"] == "grapple_point")
    {
      const float x = jsonObj["x"].asFloat();
      const float y = jsonObj["y"].asFloat();

      std::cout << "Found grapple point at: " << x << ", " << y << std::endl;
      createGrapplePoint(worldId, vec2(x, WORLD_HEIGHT_PX - y));
    }
  }

  // Process enemy spawns
  for (const auto &[id, zone] : spawnZones)
  {
    if (spawnPoints.find(id) != spawnPoints.end())
    {
      std::vector<vec2> points = spawnPoints[id];
      ENEMY_TYPES currEnemyType;

      switch (enemyType[id][0])
      {
      case 'S':
        currEnemyType = SWARM;
        break;
      case 'C':
        currEnemyType = COMMON;
        break;
      case 'O':
        currEnemyType = OBSTACLE;
        break;
      }

      int quantity = enemyQuantity[id];

      ivec2 bottom_left = ivec2(zone[0].x / TILE_WIDTH, zone[0].y / TILE_HEIGHT);
      ivec2 top_right = ivec2(zone[1].x / TILE_WIDTH, zone[1].y / TILE_HEIGHT);

      if (currEnemyType == OBSTACLE && points.size() == 2)
      {
        vec2 start = points[0];
        vec2 end = points[1];
        ivec2 spawn_location = ivec2(start.x / TILE_WIDTH, start.y / TILE_HEIGHT);
        ivec2 obstacle_patrol_bottom_left = ivec2(start.x / TILE_WIDTH, start.y / TILE_HEIGHT);
        ivec2 obstacle_patrol_top_right = ivec2(end.x / TILE_WIDTH, end.y / TILE_HEIGHT);
        insertToSpawnMap(bottom_left, top_right, currEnemyType, quantity, spawn_location, obstacle_patrol_bottom_left, obstacle_patrol_top_right);
      }
      else if (points.size() == 1)
      {
        vec2 point = points[0];
        ivec2 spawn_location = ivec2(point.x / TILE_WIDTH, point.y / TILE_HEIGHT);
        insertToSpawnMap(bottom_left, top_right, currEnemyType, quantity, spawn_location, ivec2(0, 0), ivec2(0, 0));
      }
    }
  }

  if (!spawnpoint_found)
  {
    std::cerr << "No spawnpoint found in map file." << std::endl;
    return false;
  }

  if (!goalzone_found)
  {
    std::cerr << "No goalzone found in map file." << std::endl;
    return false;
  }

  return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game(int level)
{
  // Figure out what we're using for each level
  std::tuple<std::string, TEXTURE_ASSET_ID, MUSIC> level_specs = levelMap.find(current_level)->second;
  std::string level_path = std::get<0>(level_specs);
  TEXTURE_ASSET_ID level_texture = std::get<1>(level_specs);
  MUSIC level_music = std::get<2>(level_specs);

  std::cout << "Restarting..." << std::endl;

  // Debugging for memory/component leaks
  registry.list_all_components();

  // Reset the game speed
  current_speed = 1.f;

  // Clear spawn map
  spawnMap.clear();


  player_reached_finish_line = false;
  timer_game_end_screen = TIMER_GAME_END;
  num_enemies_to_kill = countEnemiesOnLevel();
  hp = PLAYER_STARTING_HP;
  enemies_killed = 0;
  time_elapsed = 0;
  time_granularity = TIME_GRANULARITY;
  max_towers = MAX_TOWERS_START;
  next_enemy_spawn = 0;
  enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;
  if (grappleActive)
  {
    removeGrapple();
    grappleActive = false;
  }

  // remove all box2d bodies
  while (registry.physicsBodies.entities.size() > 0)
  {
    PhysicsBody &physicsBody = registry.physicsBodies.get(registry.physicsBodies.entities.back());
    b2DestroyBody(physicsBody.bodyId);
    registry.physicsBodies.remove(registry.physicsBodies.entities.back());
  }

  while (registry.motions.entities.size() > 0)
  {
    registry.remove_all_components_of(registry.motions.entities.back());
  }

  while (registry.lines.entities.size() > 0)
  {
    registry.remove_all_components_of(registry.lines.entities.back());
  }

  if (registry.players.entities.size() > 0)
  {
    // clear player-related stuff.
    Entity &playerEntity = registry.players.entities.back();
    registry.remove_all_components_of(playerEntity);
  }

  if (registry.goalZones.entities.size() > 0)
  {
    // clear goalZone
    Entity &goalEntity = registry.goalZones.entities.back();
    registry.remove_all_components_of(goalEntity);
  }

  while (registry.backgroundLayers.entities.size() > 0)
  {
    registry.backgroundLayers.remove(registry.backgroundLayers.entities.back());
  }

  while (registry.playerRotatableLayers.entities.size() > 0)
  {
    registry.playerRotatableLayers.remove(registry.playerRotatableLayers.entities.back());
  }

  while (registry.playerNonRotatableLayers.entities.size() > 0)
  {
    registry.playerNonRotatableLayers.remove(registry.playerNonRotatableLayers.entities.back());
  }

  while (registry.playerTopLayer.entities.size() > 0)
  {
    registry.playerTopLayer.remove(registry.playerTopLayer.entities.back());
  }

  while (registry.playerMidLayer.entities.size() > 0)
  {
    registry.playerMidLayer.remove(registry.playerMidLayer.entities.back());
  }

  while (registry.playerBottomLayer.entities.size() > 0)
  {
    registry.playerBottomLayer.remove(registry.playerBottomLayer.entities.back());
  }

  while (registry.runAnimations.entities.size() > 0)
  {
    registry.runAnimations.remove(registry.runAnimations.entities.back());
  }

  while (registry.idleAnimations.entities.size() > 0)
  {
    registry.idleAnimations.remove(registry.idleAnimations.entities.back());
  }

  // stop all sounds playing currently
  for (int i = 0; i < 8; i++) {
      Mix_HaltChannel(i);
  }

  int grid_line_width = GRID_LINE_WIDTH_PX;

  load_level(level_path);

  // create grid lines if they do not already exist
  if (grid_lines.size() == 0)
  {
    // vertical lines
    int cell_width = GRID_CELL_WIDTH_PX;
    int numVerticalLines = WORLD_WIDTH_PX / GRID_CELL_WIDTH_PX;
    for (int col = 0; col < numVerticalLines + 1; col++)
    {
      // width of 2 to make the grid easier to see
      grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, WORLD_HEIGHT_PX)));
    }

    // horizontal lines
    int cell_height = GRID_CELL_HEIGHT_PX;
    int numHorizontalLines = WORLD_HEIGHT_PX / GRID_CELL_HEIGHT_PX;
    for (int row = 0; row < numHorizontalLines + 1; row++)
    {
      // width of 2 to make the grid easier to see
      grid_lines.push_back(createGridLine(vec2(0, row * cell_height), vec2(WORLD_WIDTH_PX, grid_line_width)));
    }
  }

  createScreenElements();

  // Room dimensions
  const float roomWidth = WORLD_WIDTH_PX;
  const float roomHeight = WORLD_HEIGHT_PX;
  const float wallThickness = 0.5f; // half-width for SetAsBox

  // Create room boundaries
  b2BodyId floorId = create_horizontal_wall(worldId, roomWidth / 2, 0.0f, roomWidth);          // Floor
  b2BodyId ceilingId = create_horizontal_wall(worldId, roomWidth / 2, roomHeight, roomWidth);  // Ceiling
  b2BodyId leftWallId = create_vertical_wall(worldId, 0.0f, roomHeight / 2, roomHeight);       // Left Wall
  b2BodyId rightWallId = create_vertical_wall(worldId, roomWidth, roomHeight / 2, roomHeight); // Right Wall

  createBackgroundLayer(TEXTURE_ASSET_ID::BACKGROUND);
  createLevelTextureLayer(level_texture);

  // turn the tunes back on
  if (Mix_PausedMusic())
  {
    Mix_ResumeMusic();
  }
  else
  {
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    playMusic(level_music);
  }

  // reactivate the game
  game_active = true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions(float elapsed_ms)
{

  // This is mostly a repurposing of collision handling implementation from A1
  ComponentContainer<Collision> &collision_container = registry.collisions;
  for (uint i = 0; i < collision_container.components.size(); i++)
  {
    Entity entity = collision_container.entities[i];
    Collision &collision = collision_container.components[i];
    Entity other = collision.other; // the other entity in the collision

    // Player - Enemy Collision
    if ((registry.enemies.has(entity) && registry.players.has(other)) ||
        (registry.enemies.has(other) && registry.players.has(entity)))
    {

      if (registry.enemies.has(entity))
      {

        // Figure out the position, velocity characteristics of player and enemy
        Entity enemyEntity = entity;
        Enemy &enemyComponent = registry.enemies.get(enemyEntity);
        Entity playerEntity = other;
        PhysicsBody &enemyPhys = registry.physicsBodies.get(enemyEntity);
        b2BodyId enemyBodyId = enemyPhys.bodyId;
        PhysicsBody &playerPhys = registry.physicsBodies.get(playerEntity);
        b2BodyId playerBodyId = playerPhys.bodyId;

        // For now we'll base everything entirely on speed.
        // Handling based on whether player comes out on top in this collision
        if (collision.player_wins_collision && enemyComponent.destructable)
        {
          b2DestroyBody(enemyBodyId);
          registry.remove_all_components_of(enemyEntity);
          playSoundEffect(FX::FX_DESTROY_ENEMY);
          enemies_killed++;

          handleRamsterVoicelines();
        }
        // Otherwise player takes dmg (just loses pts for now) and we freeze the enemy momentarily.
        // If the enemy is still frozen, player will not be punished.
        else if (enemyComponent.freeze_time <= 0)
        {
          enemyComponent.freeze_time = ENEMY_FREEZE_TIME_MS;
          playSoundEffect(FX::FX_DESTROY_ENEMY_FAIL);

          // Only lose HP if what we hit was NOT an obstacle
          if (enemyComponent.destructable)
          {
            hp -= 1; // small penalty for now
          }
        }
      }
      else
      {

        // Figure out the position, velocity characteristics of player and enemy
        Entity enemyEntity = other;
        Enemy &enemyComponent = registry.enemies.get(enemyEntity);
        Entity playerEntity = entity;
        PhysicsBody &enemyPhys = registry.physicsBodies.get(enemyEntity);
        b2BodyId enemyBodyId = enemyPhys.bodyId;
        PhysicsBody &playerPhys = registry.physicsBodies.get(playerEntity);
        b2BodyId playerBodyId = playerPhys.bodyId;

        // Handling based on whether player comes out on top in this collision
        if (collision.player_wins_collision && enemyComponent.destructable)
        {
          b2DestroyBody(enemyBodyId);
          registry.remove_all_components_of(other);
          playSoundEffect(FX::FX_DESTROY_ENEMY);
          enemies_killed++;

		  handleRamsterVoicelines();
        }
        // Otherwise player takes dmg (just loses pts for now) and we freeze the enemy momentarily.
        // If the enemy is still frozen, player will not be punished.
        else if (enemyComponent.freeze_time <= 0)
        {
          enemyComponent.freeze_time = ENEMY_FREEZE_TIME_MS;
          playSoundEffect(FX::FX_DESTROY_ENEMY_FAIL);

          // Only lose HP if what we hit was NOT an obstacle
          if (enemyComponent.destructable)
          {
            hp -= 1; // small penalty for now
          }
        }
      }
    }
  }
  // Remove all collisions from this simulation step
  registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
  return bool(glfwWindowShouldClose(window));
}

void WorldSystem::update_isGrounded()
{
  Entity playerEntity = registry.players.entities[0];
  PhysicsBody &phys = registry.physicsBodies.get(playerEntity);
  b2BodyId bodyId = phys.bodyId;

  // calculate if ball is grounded or not.
  int num_contacts = b2Body_GetContactCapacity(bodyId);
  bool &isGroundedRef = registry.playerPhysics.get(playerEntity).isGrounded;

  if (num_contacts == 0)
  {
    isGroundedRef = false;
    return;
  }

  b2ContactData *contactData = new b2ContactData[num_contacts];
  b2Body_GetContactData(bodyId, contactData, num_contacts);

  // get the ball's shape id.
  // The # shapes should always be 1, since the player is initialized as a singular ball shape!
  int player_num_shapes = b2Body_GetShapeCount(bodyId);
  b2ShapeId *shapeArray = new b2ShapeId[player_num_shapes];
  b2Body_GetShapes(bodyId, shapeArray, player_num_shapes);

  b2ShapeId player_shape = shapeArray[0];

  for (int i = 0; i < num_contacts; i++)
  {
    b2ContactData contact = contactData[i];

    // if the collision involves the player.
    if ((contact.shapeIdA.index1 == player_shape.index1 || contact.shapeIdB.index1 == player_shape.index1))
    {
      b2Manifold manifold = contact.manifold;
      b2Vec2 normal = manifold.normal;

      if (normal.y >= 0.15f)
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
void WorldSystem::handle_movement()
{

  // first, update states.
  int state = glfwGetKey(window, GLFW_KEY_E);

  for (int i = 0; i < PLAYER_CONTROL_KEYS.size(); i++)
  {
    int key = PLAYER_CONTROL_KEYS[i];
    int action = glfwGetKey(window, key);

    // set the keyState
    if (action == GLFW_PRESS)
    {
      keyStates[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
      keyStates[key] = false;
    }
  }

  b2Vec2 nonjump_movement_force = {0, 0};
  b2Vec2 jump_impulse = {0, 0};
  const float forceMagnitude = BALL_GROUNDED_MOVEMENT_FORCE;
  const float jumpImpulseMagnitude = BALL_JUMP_IMPULSE;

  // Determine impulse direction based on key pressed
  if (keyStates[GLFW_KEY_W])
  {
    // nonjump_movement_force = { 0, forceMagnitude };
    if (grappleActive)
    {
      for (Entity grappleEntity : registry.grapples.entities)
      {
        Grapple &grapple = registry.grapples.get(grappleEntity);
        float curLen = b2DistanceJoint_GetCurrentLength(grapple.jointId);
        if (curLen >= 50.0f)
        {
          b2DistanceJoint_SetLength(grapple.jointId, curLen - GRAPPLE_DETRACT_W);
        }
      }
    }
  }
  else if (keyStates[GLFW_KEY_A])
  {
    if (grappleActive)
    {
      // Speed boost for grapple
      nonjump_movement_force = {-forceMagnitude * 3, 0};
    }
    else
    {
      nonjump_movement_force = {-forceMagnitude, 0};
    }
  }
  else if (keyStates[GLFW_KEY_S])
  {
    // nonjump_movement_force = { 0, -forceMagnitude };
  }
  else if (keyStates[GLFW_KEY_D])
  {
    if (grappleActive)
    {
      // Speed boost for grapple
      nonjump_movement_force = {forceMagnitude * 3, 0};
    }
    else
    {
      nonjump_movement_force = {forceMagnitude, 0};
    }
  }

  // jump is set seperately, since it can be used in conjunction with the movement keys.
  if (keyStates[GLFW_KEY_SPACE])
  {
    // Jump: apply a strong upward impulse
    jump_impulse = {0, jumpImpulseMagnitude};
  }

  // Apply impulse if non-zero.
  if (nonjump_movement_force != b2Vec2_zero || jump_impulse != b2Vec2_zero)
  {
    // Assuming registry.players.entities[0] holds the player entity.
    if (!registry.players.entities.empty())
    {
      Entity playerEntity = registry.players.entities[0];

      if (registry.physicsBodies.has(playerEntity))
      {
        PhysicsBody &phys = registry.physicsBodies.get(playerEntity);
        b2BodyId bodyId = phys.bodyId;

        // make sure player is grounded.
        bool isGrounded = registry.playerPhysics.get(playerEntity).isGrounded;

        // if jump is registered, it should override any other force being applied.
        if (jump_impulse != b2Vec2_zero && isGrounded)
        {
          b2Body_ApplyLinearImpulseToCenter(bodyId, jump_impulse, true);
        }
        else if (nonjump_movement_force != b2Vec2_zero)
        {
          float multiplier = 1.0f;

          if (!isGrounded)
          {
            multiplier = BALL_AIR_STRAFE_FORCE_MULTIPLIER;
          }

          // apply force slightly above center of mass to make ball spin.
          // don't get the reference of the position, we don't want to alter the value.
          b2Vec2 bodyPosition = b2Body_GetPosition(bodyId);
          bodyPosition.y += 2.f;
          b2Body_ApplyForce(bodyId, nonjump_movement_force * multiplier, bodyPosition, true);
        }
      }
    }
  }
}

// on key callback
void WorldSystem::on_key(int key, int scancode, int action, int mod)
{
  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  if (!game_active)
  {
    /* DISABLED - REPLACED WITH MOUSE INPUT HANDLING.
    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
      restart_game(current_level);
    }
    */
    return; // ignore all other inputs when game is inactive
  }

  // Exit game with ESC
  if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
  {
    // Playing screen - pauses game
    if (currentScreen.current_screen == "PLAYING")
    {
      currentScreen.current_screen = "PAUSE";
      // freezeMovements();
      return;
    }
    /* DISABLED - REPLACED WITH MOUSE INPUT HANDLING.
    // Pause screen - resume game
    if (currentScreen.current_screen == "PAUSE")
    {
      currentScreen.current_screen = "PLAYING";
      return;
    }
    // Menu and End of Game screen - exits game
    if (currentScreen.current_screen == "MAIN MENU" || currentScreen.current_screen == "VICTORY" || currentScreen.current_screen == "DEFEAT")
    {
      close_window();
    }
    */
  }

  // Reset game when R is released
  /* DISABLED. REPLACED WITH MOUSE INPUT HANDLING.
  if (action == GLFW_RELEASE && key == GLFW_KEY_R)
  {
    // Pause and End of Game screen - restarts game
    if (currentScreen.current_screen == "PAUSE" || currentScreen.current_screen == "VICTORY" || currentScreen.current_screen == "DEFEAT")
    {
      currentScreen.current_screen = "PLAYING";
      int w, h;
      glfwGetWindowSize(window, &w, &h);
      restart_game(current_level);
    }
  }
  */
  // Select level - only active on MAIN MENU screen
  /* DISABLED. REPLACED WITH MOUSE INPUT HANDLING.
  if (currentScreen.current_screen == "MAIN MENU")
  {
    // Level keys 1-9
    if (action == GLFW_RELEASE && key == GLFW_KEY_1)
    {
      levelHelper(1, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_2)
    {
      levelHelper(2, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_3)
    {
      levelHelper(3, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_4)
    {
      levelHelper(4, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_5)
    {
      levelHelper(5, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_6)
    {
      levelHelper(6, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_7)
    {
      levelHelper(7, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_8)
    {
      levelHelper(8, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_9)
    {
      levelHelper(9, currentScreen);
    }
    // Increment or decrement selected level by 1
    if (action == GLFW_RELEASE && key == GLFW_KEY_UP)
    {
      levelHelper(current_level + 1, currentScreen);
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_DOWN)
    {
      levelHelper(current_level - 1, currentScreen);
    }
  }
  */

  // ENTER key press handling
  /* DISABLED. REPLACED WITH MOUSE INPUT HANDLING.
  if (action == GLFW_RELEASE && key == GLFW_KEY_ENTER)
  {

    // Main menu screen - Loads the selected level and starts the game
    if (currentScreen.current_screen == "MAIN MENU")
    {
      currentScreen.current_screen = "PLAYING";
      restart_game(current_level);
      return;
    }
    // Pause and End of Game screen - back to main menu
    if (currentScreen.current_screen == "PAUSE" || currentScreen.current_screen == "VICTORY" || currentScreen.current_screen == "DEFEAT")
    {
      currentScreen.current_screen = "MAIN MENU";
      restart_game(current_level);
      return;
    }
  }
  */

  // Debug toggle with D
  if (key == GLFW_KEY_P && action == GLFW_RELEASE)
  {
    debugging.in_debug_mode = !debugging.in_debug_mode;
  }
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{

  // record the current mouse position
  mouse_pos_x = mouse_position.x;
  mouse_pos_y = mouse_position.y;
  // std::cout << "mouse coordinate position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  if (!game_active)
  {
    return;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
  {

    // Get mouse position and convert to world coordinates.
    vec2 mouseScreenPos = {mouse_pos_x, mouse_pos_y};
    vec2 worldMousePos = screenToWorld(mouseScreenPos);

    /*
    std::cout << "Mouse clicked at world position: ("
                << worldMousePos.x << ", " << worldMousePos.y << ")" << std::endl;
    std::cout << "Corresponding tile: ("
                << worldMousePos.x / GRID_CELL_WIDTH_PX << ", " << worldMousePos.y / GRID_CELL_HEIGHT_PX << ")" << std::endl;
    */

    // For the playing screen specifically, mouse controls grapple
    if (currentScreen.current_screen == "PLAYING")
    {
      // Find the grapple point closest to the click that is within the threshold.
      GrapplePoint *selectedGp = nullptr;
      float bestDist = GRAPPLE_ATTACH_ZONE_RADIUS; // distance threshold

      for (Entity gpEntity : registry.grapplePoints.entities)
      {
        GrapplePoint &gp = registry.grapplePoints.get(gpEntity);
        float dist = length(gp.position - worldMousePos);
        if (dist < bestDist)
        {
          bestDist = dist;
          selectedGp = &gp;
        }
      }

      // Deactivate all grapple enemies_killed.
      for (Entity gpEntity : registry.grapplePoints.entities)
      {
        GrapplePoint &gp = registry.grapplePoints.get(gpEntity);
        gp.active = false;
      }

      // If a valid grapple point is found, mark it as active.
      if (selectedGp != nullptr)
      {
        selectedGp->active = true;
        std::cout << "Selected grapple point at ("
                  << selectedGp->position.x << ", " << selectedGp->position.y
                  << ") with active = " << selectedGp->active << std::endl;
      }

      // Now, if no grapple is currently attached and we found an active point, attach the grapple.
      if (!grappleActive && selectedGp != nullptr)
      {
        attachGrapple();
        playSoundEffect(FX::FX_GRAPPLE);
      }
      else if (grappleActive)
      {
        removeGrapple();
        playSoundEffect(FX::FX_GRAPPLE);
        grappleActive = false;
      }
    }
    // Every other screen, mouse deals with button presses.
    else
    {

      // Get camera to center on
      Camera camera = registry.cameras.get(registry.cameras.entities[0]);
      vec2 center = vec2(camera.position.x, camera.position.y);

      // std::cout << "MOUSE POSITION: " << worldMousePos.x << ", " << worldMousePos.y << std::endl;
      // std::cout << "CENTER: " << center.x << ", " << center.y << std::endl;

      // Iterate over all buttons to figure out which one was clicked
      std::vector<Entity> buttonEntities = registry.buttons.entities;

      for (Entity buttonEntity : buttonEntities)
      {

        ScreenElement screenElement = registry.screenElements.get(buttonEntity);

        // We're only concerned about buttons for the screen we're currently on
        if (screenElement.screen == currentScreen.current_screen)
        {

          Button button = registry.buttons.get(buttonEntity);

          // Figure out LEFT, RIGHT, TOP, BOTTOM of button
          float left = center.x + screenElement.boundaries[0];
          float bottom = center.y + screenElement.boundaries[1];
          float right = center.x + screenElement.boundaries[2];
          float top = center.y + screenElement.boundaries[3];

          // std::cout << "BUTTON POSITION: " << screenElement.position.x + center.x << ", " << screenElement.position.y + camera.position.y << std::endl;
          // std::cout << camera.position.x << ", " << camera.position.y << std::endl;
          // std::cout << button.function << " BUTTON BOUNDS: " << "X: " << left << ", " << right << " Y: " << bottom << ", " << top << std::endl;

          // If the mouse click lies within the button boundaries, it means we clicked it
          if (worldMousePos.x > left && worldMousePos.x < right && worldMousePos.y > bottom && worldMousePos.y < top)
          {

            // std::cout << "!!!!!! BUTTON PRESSED !!!!!!" << button.function << std::endl;

            // Handle the button press
            handleButtonPress(buttonEntity);
            break;
          }
        }
      }
    }
  }
}

vec2 WorldSystem::screenToWorld(vec2 mouse_screen)
{
  int win_w, win_h;
  glfwGetWindowSize(window, &win_w, &win_h);

  // Flip Y: mouse origin is top-left, OpenGL is bottom-left
  mouse_screen.y = win_h - mouse_screen.y;

  // Grab viewport from renderer (letterboxed area)
  int vx = renderer->screen_viewport_x;
  int vy = renderer->screen_viewport_y;
  int vw = renderer->screen_viewport_w;
  int vh = renderer->screen_viewport_h;

  // Check if click is outside the visible game area
  if (mouse_screen.x < vx || mouse_screen.x > vx + vw ||
      mouse_screen.y < vy || mouse_screen.y > vy + vh)
  {
    return {-1, -1}; // outside the viewport
  }

  // Normalize to [0,1] within the viewport
  float norm_x = (mouse_screen.x - vx) / vw;
  float norm_y = (mouse_screen.y - vy) / vh;

  // Map to virtual game coordinates
  float virtual_x = norm_x * VIEWPORT_WIDTH_PX;  // 1200.f;
  float virtual_y = norm_y * VIEWPORT_HEIGHT_PX; // 900.f;

  // Offset from screen center (in virtual resolution)
  float offset_x = virtual_x - VIEWPORT_WIDTH_PX / 2;  // 1200.f / 2.f;
  float offset_y = virtual_y - VIEWPORT_HEIGHT_PX / 2; // 900.f / 2.f;

  // Add camera position to get world-space coordinate
  for (Entity cameraEntity : registry.cameras.entities)
  {
    Camera &camera = registry.cameras.get(cameraEntity);

    vec2 worldPos;
    worldPos.x = offset_x + camera.position.x;
    worldPos.y = offset_y + camera.position.y;

    std::cout << "mouse coordinate world pos: " << worldPos.x << ", " << worldPos.y << std::endl;
    return worldPos;
  }

  return {0.f, 0.f}; // fallback if no camera
}

void WorldSystem::attachGrapple()
{
  Entity playerEntity = registry.players.entities[0];

  // Retrieve the ball's physics body
  PhysicsBody &ballBody = registry.physicsBodies.get(playerEntity);
  b2BodyId ballBodyId = ballBody.bodyId;
  b2Vec2 ballPos = b2Body_GetPosition(ballBodyId);

  Entity activeGrapplePointEntity;
  b2BodyId activeGrappleBodyId;
  bool foundActive = false;

  // Loop through all grapple enemies_killed and find the active one
  for (Entity gpEntity : registry.grapplePoints.entities)
  {
    GrapplePoint &gp = registry.grapplePoints.get(gpEntity);
    std::cout << gp.position.x << " " << gp.position.y << " " << gp.active << std::endl;
    if (gp.active)
    {
      activeGrapplePointEntity = gpEntity;
      activeGrappleBodyId = gp.bodyId;
      foundActive = true;
      break; // Stop once we find the first active grapple point
    }
  }

  // If no active grapple enemies_killed were found, exit early
  if (!foundActive)
  {
    return;
  }

  b2Vec2 grapplePos = b2Body_GetPosition(activeGrappleBodyId);

  // Compute the distance between the ball and the grapple point
  float distance = sqrtf((grapplePos.x - ballPos.x) * (grapplePos.x - ballPos.x) +
                         (grapplePos.y - ballPos.y) * (grapplePos.y - ballPos.y));

  // Attach the grapple if within range
  if (distance <= GRAPPLE_MAX_LENGTH)
  {
    createGrapple(worldId, ballBodyId, activeGrappleBodyId, distance);
    grappleActive = true;
  }
}

void WorldSystem::checkGrappleGrounded()
{
  if (grappleActive)
  {
    if (!registry.players.entities.empty())
    {
      Entity playerEntity = registry.players.entities[0];

      if (registry.physicsBodies.has(playerEntity))
      {
        PhysicsBody &phys = registry.physicsBodies.get(playerEntity);
        b2BodyId bodyId = phys.bodyId;

        // check if the player is grounded
        bool isGrounded = registry.playerPhysics.get(playerEntity).isGrounded;
        Grapple grapple;
        for (Entity grappleEntity : registry.grapples.entities)
        {
          grapple = registry.grapples.get(grappleEntity);
        }
        float curLen = b2DistanceJoint_GetCurrentLength(grapple.jointId);
        if (isGrounded)
        {
          b2DistanceJoint_EnableSpring(grapple.jointId, true);
          b2DistanceJoint_SetSpringHertz(grapple.jointId, GRAPPLE_HERTZ_GROUNDED);
          b2DistanceJoint_SetSpringDampingRatio(grapple.jointId, GRAPPLE_DAMPING_GROUNDED);
          b2DistanceJoint_SetLength(grapple.jointId, curLen - GRAPPLE_DETRACT_GROUNDED);
        }
        else
        {
          b2DistanceJoint_EnableSpring(grapple.jointId, false);
        }
      }
    }
  }
}

void WorldSystem::handleEnemySpawning(ENEMY_TYPES enemy_type, int quantity, ivec2 gridPosition, ivec2 grid_patrol_point_a, ivec2 grid_patrol_point_b)
{
  // Create specified number of enemies by iterating
  for (int i = 0; i < quantity; i++)
  {
    createEnemy(
        worldId,
        vec2((gridPosition.x + 0.5 + 0.05 * i) * GRID_CELL_WIDTH_PX,
             (gridPosition.y + 0.5) * GRID_CELL_HEIGHT_PX),
        enemy_type,
        vec2((grid_patrol_point_a.x + 0.5) * GRID_CELL_WIDTH_PX, (grid_patrol_point_a.y + 0.5) * GRID_CELL_HEIGHT_PX),
        vec2((grid_patrol_point_b.x + 0.5) * GRID_CELL_WIDTH_PX, (grid_patrol_point_b.y + 0.5) * GRID_CELL_HEIGHT_PX));
  }
}

// NOTE THAT ALL POSITIONS ARE GRID COORDINATES!!!
// Takes:
// - Enemy Spawn Area
// - Enemy type/number to spawn
// - Location to spawn enemy
// - Patrol area if it's an obstacle
// Returns:
// - Handles enemy spawning according to specs.
void WorldSystem::insertToSpawnMap(ivec2 bottom_left, ivec2 top_right,
                                   ENEMY_TYPES enemy_type, int num_enemies, ivec2 spawn_location,
                                   ivec2 obstacle_patrol_point_a, ivec2 obstacle_patrol_point_b)
{

  // Prep key from "spawn trigger area"
  std::vector<int> mapKey = {bottom_left.x, bottom_left.y, top_right.x, top_right.y};

  // Prep value from other inputs
  std::tuple<ENEMY_TYPES,
             int,
             bool,
             bool,
             std::vector<int>,
             std::vector<int>>
      mapValue = {enemy_type,
                  num_enemies,
                  false,
                  false,
                  {spawn_location.x, spawn_location.y},
                  {obstacle_patrol_point_a.x, obstacle_patrol_point_a.y,
                   obstacle_patrol_point_b.x, obstacle_patrol_point_b.y}};

  // Insert to map
  spawnMap.insert({mapKey, mapValue});
}

bool WorldSystem::checkPlayerReachedArea(ivec2 area_bottom_left, ivec2 area_top_right)
{
  // Get player
  Entity player = registry.players.entities[0];
  Motion playerMotion = registry.motions.get(player);

  // trigger area bounds
  int min_x = area_bottom_left.x;
  int min_y = area_bottom_left.y;
  int max_x = area_top_right.x;
  int max_y = area_top_right.y;

  // Player location in terms of grid coordinates
  ivec2 playerLocation = ivec2(playerMotion.position.x / GRID_CELL_WIDTH_PX, playerMotion.position.y / GRID_CELL_HEIGHT_PX);

  // check that player location is inside the trigger area
  bool player_inside_trigger_area = playerLocation.x >= min_x && playerLocation.y >= min_y && playerLocation.x <= max_x && playerLocation.y <= max_y;

  return player_inside_trigger_area;
}

void WorldSystem::levelHelper(int level, CurrentScreen &currentScreen)
{
  // Player sent to intro if they select level 1
  if (level == 1)
  {
    currentScreen.current_screen = "STORY INTRO";
    playMusic(MUSIC::MENU);
  }
  else if (level <= levelMap.size() && level > 0)
  {

    current_level = level;

    currentScreen.current_screen = "PLAYING";
    restart_game(level);
  }
}

int WorldSystem::countEnemiesOnLevel()
{

  int num_enemies = 0;

  for (auto &i : spawnMap)
  {

    std::vector<int> spawnTile = i.first;
    std::tuple<ENEMY_TYPES, int, bool, bool, std::vector<int>, std::vector<int>> &enemyDataTuple = i.second;
    ENEMY_TYPES enemyType = std::get<0>(enemyDataTuple);
    int quantity = std::get<1>(enemyDataTuple);
    bool &hasPlayerReachedTile = std::get<2>(enemyDataTuple);
    bool &hasEnemyAlreadySpawned = std::get<3>(enemyDataTuple);
    std::vector<int> spawnPosition = std::get<4>(enemyDataTuple);
    std::vector<int> patrolRange = std::get<5>(enemyDataTuple);

    // This'll need to be changed if we add more indestructible enemies.
    if (enemyType != OBSTACLE)
    {
      num_enemies += quantity;
    }
  }

  return num_enemies;
}

void WorldSystem::handleGameover(CurrentScreen &currentScreen)
{

  // If player HP reaches 0, game ends.
  if (hp <= 0)
  {
    currentScreen.current_screen = "DEFEAT";
  }
  // If player reached finish line AND killed all enemies, game ends.
  else if (player_reached_finish_line)
  {
    if (timer_game_end_screen <= 0)
    {
      // Make sure to show conclusion sequence if finished level 12
      if (current_level == 12)
      {
        currentScreen.current_screen = "STORY CONCLUSION";
        playMusic(MUSIC::MENU);
      }
      else
      {
        currentScreen.current_screen = "VICTORY";
      }
    }
    else
    {
      timer_game_end_screen -= time_elapsed;
    }
  }
}

// NOT NEEDED IF WE JUST FREEZE PHYSICS!!! (in fact it's better if we froze physics as original velocity preserved
void WorldSystem::freezeMovements()
{

  // Get enemy entities
  auto &enemy_registry = registry.enemies; // list of enemy entities stored in here

  // Get player entity
  Entity playerEntity = registry.players.entities[0];
  Motion &playerMotion = registry.motions.get(playerEntity);
  b2BodyId player_id = registry.physicsBodies.get(playerEntity).bodyId;

  // freeze the player
  b2Body_SetLinearVelocity(player_id, b2Vec2_zero);
  playerMotion.velocity = vec2(0, 0);

  // freeze the enemies
  // Iterate over each enemy and implement basic logic as commented above.
  for (int i = 0; i < enemy_registry.entities.size(); i++)
  {

    // Figure out enemy details
    Entity enemyEntity = enemy_registry.entities[i];
    Motion &enemyMotion = registry.motions.get(enemyEntity);
    Enemy &enemyComponent = registry.enemies.get(enemyEntity);

    // Get Box2D Speed
    b2BodyId enemy_id = registry.physicsBodies.get(enemyEntity).bodyId;

    // freeze the enemy
    b2Body_SetLinearVelocity(enemy_id, b2Vec2_zero);
    enemyMotion.velocity = vec2(0, 0);
  }
}

// Handles button press based on function.
// LLNOTE: SHOULD HAVE A CASE FOR EACH BUTTON CREATED IN createScreenElements().
void WorldSystem::handleButtonPress(Entity buttonEntity)
{

  // Current Screen
  Entity currScreenEntity = registry.currentScreen.entities[0];
  CurrentScreen &currentScreen = registry.currentScreen.get(currScreenEntity);

  // Get function of button
  Button buttonComponent = registry.buttons.get(buttonEntity);
  std::string function = buttonComponent.function;

  if (function == "LEVEL BUTTON")
  {
    // Level buttons have an extra level component that we can use to select the level with
    Level level = registry.levels.get(buttonEntity);

    levelHelper(level.level, currentScreen);
  }
  else if (function == "STORY FRAME BUTTON")
  {

    // This button is a bit of a special case, as it could be any of the frames, but we only want to deal with the smallest one in the sequence. So we'll be looking
    // for the entity representing the EARLIEST frame as opposed to the button entity itself.

    // Get the smallest story frame on current screen
    Entity storyFrameToHandle;
    int lowest_frame = 9999;

    for (Entity entity : registry.storyFrames.entities)
    {

      // We only want story frames for the current screen
      ScreenElement screenElement = registry.screenElements.get(entity);
      if (screenElement.screen == currentScreen.current_screen)
      {

        // If this story frame's the lowest in the sequence then we want to render it.
        StoryFrame storyFrame = registry.storyFrames.get(entity);
        if (storyFrame.frame < lowest_frame)
        {
          storyFrameToHandle = entity;
          lowest_frame = storyFrame.frame;
        }
      }
    }

    // Get the storyframe for this entity
    StoryFrame storyFrame = registry.storyFrames.get(storyFrameToHandle);
    std::cout << "STORY FRAME: " << storyFrame.frame << "MAX FRAME: " << storyFrame.max_frame << std::endl;

    // If it's the last frame in the sequence we go to the next screen
    if (storyFrame.frame == storyFrame.max_frame)
    {

      if (currentScreen.current_screen == "STORY INTRO")
      {

        current_level = 1;
        currentScreen.current_screen = "PLAYING";
        restart_game(1);
      }

      else if (currentScreen.current_screen == "STORY CONCLUSION")
      {
        currentScreen.current_screen = "GAME COMPLETE";
        playMusic(MUSIC::MENU);
      }
    }
    // Otherwise we delete the EARLIEST story frame so the remaining frames get rendered
    else
    {
      registry.remove_all_components_of(storyFrameToHandle);
    }
  }
  else if (function == "NEXT LEVEL")
  {
    levelHelper(current_level + 1, currentScreen);
  }
  else if (function == "EXIT GAME")
  {
    close_window();
  }
  else if (function == "START GAME")
  {

    currentScreen.current_screen = "LEVEL SELECT";
  }
  else if (function == "RESUME")
  {

    currentScreen.current_screen = "PLAYING";
  }
  else if (function == "RESTART")
  {

    int w, h;
    currentScreen.current_screen = "PLAYING";
    glfwGetWindowSize(window, &w, &h);
    restart_game(current_level);
  }
  else if (function == "MAIN MENU")
  {

    currentScreen.current_screen = "MAIN MENU";
    restart_game(current_level);
  }
}

// LLNOTE: FOR CODE READABILITY, ALL OF THE SCREEN ELEMENT AND BUTTON CREATIONS SHOULD BE IN HERE.
// create screens if they do not already exist
void WorldSystem::createScreenElements()
{

  if (registry.screens.entities.size() == 0)
  {

    // MAIN MENU ////////////////////////////////////////////////////////

    // Title
    createScreenElement(
        "MAIN MENU", TEXTURE_ASSET_ID::TITLE_MENU,
        900, 400,
        vec2(0, 100));

    // Start
    createButton(
        "START GAME", "MAIN MENU", TEXTURE_ASSET_ID::BUTTON_START,
        256, 128,
        vec2(-200, -200));

    // Exit
    createButton(
        "EXIT GAME", "MAIN MENU", TEXTURE_ASSET_ID::BUTTON_EXITGAME,
        256, 128,
        vec2(200, -200));

    // LEVEL SELECT ////////////////////////////////////////////////////////
    // Row 1
    createLevelButton(
        1, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL1,
        128, 128,
        vec2(-375, 250));

    createLevelButton(
        2, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL2,
        128, 128,
        vec2(-125, 250));

    createLevelButton(
        3, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL3,
        128, 128,
        vec2(125, 250));

    createLevelButton(
        4, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL4,
        128, 128,
        vec2(375, 250));

    // Row 2
    createLevelButton(
        5, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL5,
        128, 128,
        vec2(-375, 0));

    createLevelButton(
        6, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL6,
        128, 128,
        vec2(-125, 0));

    createLevelButton(
        7, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL7,
        128, 128,
        vec2(125, 0));

    createLevelButton(
        8, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL8,
        128, 128,
        vec2(375, 0));

    // Row 3
    createLevelButton(
        9, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL9,
        128, 128,
        vec2(-375, -250));

    createLevelButton(
        10, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL10,
        128, 128,
        vec2(-125, -250));

    createLevelButton(
        11, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL11,
        128, 128,
        vec2(125, -250));

    createLevelButton(
        12, "LEVEL SELECT", TEXTURE_ASSET_ID::BUTTON_LVL12,
        128, 128,
        vec2(375, -250));

    // PAUSE ////////////////////////////////////////////////////////

    // Title
    createScreenElement(
        "PAUSE", TEXTURE_ASSET_ID::TITLE_PAUSE,
        900, 400,
        vec2(0, 100));

    // Buttons
    //
    // Resume
    createButton(
        "RESUME", "PAUSE", TEXTURE_ASSET_ID::BUTTON_RESUME,
        256, 128,
        vec2(-400, -200));

    // Restart
    createButton(
        "RESTART", "PAUSE", TEXTURE_ASSET_ID::BUTTON_RESTART,
        256, 128,
        vec2(0, -200));

    // Main Menu
    createButton(
        "MAIN MENU", "PAUSE", TEXTURE_ASSET_ID::BUTTON_MAINMENU,
        256, 128,
        vec2(400, -200));

    // VICTORY ////////////////////////////////////////////////////////

    // Title
    createScreenElement(
        "VICTORY", TEXTURE_ASSET_ID::TITLE_VICTORY,
        900, 400,
        vec2(0, 100));

    // Buttons
    //
    // Main Menu
    createButton(
        "MAIN MENU", "VICTORY", TEXTURE_ASSET_ID::BUTTON_MAINMENU,
        256, 128,
        vec2(-200, -200));

    // Restart
    createButton(
        "NEXT LEVEL", "VICTORY", TEXTURE_ASSET_ID::BUTTON_LVLUP,
        256, 128,
        vec2(200, -200));

    // DEFEAT ////////////////////////////////////////////////////////

    // Title
    createScreenElement(
        "DEFEAT", TEXTURE_ASSET_ID::TITLE_DEFEAT,
        900, 400,
        vec2(0, 100));

    // Buttons
    //
    // Resume
    createButton(
        "RESUME", "DEFEAT", TEXTURE_ASSET_ID::BUTTON_RESUME,
        256, 128,
        vec2(-400, -200));

    // Restart
    createButton(
        "RESTART", "DEFEAT", TEXTURE_ASSET_ID::BUTTON_RESTART,
        256, 128,
        vec2(0, -200));

    // Main Menu
    createButton(
        "MAIN MENU", "DEFEAT", TEXTURE_ASSET_ID::BUTTON_MAINMENU,
        256, 128,
        vec2(400, -200));

    // STORY INTRO /////////////////////////////////////////////////////////

    createStoryFrame(1, 4, "STORY INTRO", TEXTURE_ASSET_ID::STORYFRAME_INTRO_1);
    createStoryFrame(2, 4, "STORY INTRO", TEXTURE_ASSET_ID::STORYFRAME_INTRO_2);
    createStoryFrame(3, 4, "STORY INTRO", TEXTURE_ASSET_ID::STORYFRAME_INTRO_3);
    createStoryFrame(4, 4, "STORY INTRO", TEXTURE_ASSET_ID::STORYFRAME_INTRO_4);

    // STORY CONCLUSION ///////////////////////////////////////////////////

    createStoryFrame(1, 3, "STORY CONCLUSION", TEXTURE_ASSET_ID::STORYFRAME_CONCLUSION_1);
    createStoryFrame(2, 3, "STORY CONCLUSION", TEXTURE_ASSET_ID::STORYFRAME_CONCLUSION_2);
    createStoryFrame(3, 3, "STORY CONCLUSION", TEXTURE_ASSET_ID::STORYFRAME_CONCLUSION_3);

    // GAME COMPLETE ///////////////////////////////////////////////////////

    // Title
    createScreenElement(
        "GAME COMPLETE", TEXTURE_ASSET_ID::TITLE_VICTORY,
        900, 400,
        vec2(0, 100));

    // Buttons
    //
    // Main Menu
    createButton(
        "MAIN MENU", "GAME COMPLETE", TEXTURE_ASSET_ID::BUTTON_MAINMENU,
        256, 128,
        vec2(0, -200));
  }
}