#pragma once
#include "common.hpp"
#include <chrono>
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Any element on the screen. Title, Label, Button, Etc. This is what gets rendered.
struct ScreenElement
{

  // Screen that the element belongs to
  std::string screen;

  // Boundaries/size of the screen element (x1, y1, x2, y2)
  // Note: primary use of this is just button hitbox but adding it as part of this component makes more sense because it'd also be render bounds.
  vec4 boundaries;

  // Camera entity for screen centering
  Entity camera;

  // Position of the screen (x, y) relative to camera (center)
  vec2 position;
};

// Indicates that a screen element is a button.
struct UIButton
{

  // Identifies the button
  std::string function;
};

// Indicates that a button has a level
struct Level
{
  int level;
};

// Indicates that this screen element is part of a story frame
struct StoryFrame
{
  int frame;
  // Total number of frames for this story sequence
  int max_frame;
};

// Screen component
// NOTE: LEGACY CODE. MIGRATED TO ScreenElement.
struct Screen
{
  std::string screen;

  // Camera entity for screen positioning
  Entity screen_center;
};

// Current Screen Component - used to track current screen.
struct CurrentScreen
{
  std::string current_screen = "MAIN MENU"; // start on main menu
};

// Player component
struct Player
{
  bool isCurrentlyRolling;
  bool isCurrentlyFlamming;
  int enemiesRecentlyDestroyed;
  float voicelineProbability;
  std::chrono::steady_clock::time_point lastVoicelineTime;
};

// Enemy component
struct Enemy
{
  // This'll be the type of enemy
  ENEMY_TYPES enemyType;
  // Because we are making obstacle types, we'll also have a "destructability" trait
  bool destructable;
  // We apply the freeze time to destructable enemies upon collision so they stop pursuing the player momentarily after a collision.
  float freeze_time;

  // Patrol boundary for obstacles (a_x, a_y), (b_x, b_y).
  vec2 movement_area_point_a;
  vec2 movement_area_point_b;
};

// All data relevant to the shape and motion of entities
struct Motion
{
  vec2 position = {0, 0}; // pixel coordinates
  float angle = 0;
  vec2 velocity = {0, 0};
  vec2 scale = {10, 10};
};

// Stucture to store collision information
struct Collision
{
  // Note, the first object is stored in the ECS container.entities
  Entity other; // the second object involved in the collision
  Collision(Entity &other) { this->other = other; };
  // Determine if player comes out on top in this collision
  bool player_wins_collision;
};

// Data structure for toggling debug mode
struct Debug
{
  bool in_debug_mode = 0;
  bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
  float darken_screen_factor = -1;
  float vignette = -1;
  float fadeout = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
  // Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine
{
  vec2 start_pos = {0, 0};
  vec2 end_pos = {10, 10}; // default to diagonal line
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
  float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
  vec3 position;
  vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
  vec3 position;
  vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
  static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
  vec2 original_size = {1, 1};
  std::vector<ColoredVertex> vertices;
  std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */
enum class TEXTURE_ASSET_ID
{
  // Numbers
  NUMBER_0 = 0,
  NUMBER_1 = NUMBER_0 + 1,
  NUMBER_2 = NUMBER_1 + 1,
  NUMBER_3 = NUMBER_2 + 1,
  NUMBER_4 = NUMBER_3 + 1,
  NUMBER_5 = NUMBER_4 + 1,
  NUMBER_6 = NUMBER_5 + 1,
  NUMBER_7 = NUMBER_6 + 1,
  NUMBER_8 = NUMBER_7 + 1,
  NUMBER_9 = NUMBER_8 + 1,

  // Red numbers
  R_NUMBER_0 = NUMBER_9 + 1,
  R_NUMBER_1 = R_NUMBER_0 + 1,
  R_NUMBER_2 = R_NUMBER_1 + 1,
  R_NUMBER_3 = R_NUMBER_2 + 1,
  R_NUMBER_4 = R_NUMBER_3 + 1,
  R_NUMBER_5 = R_NUMBER_4 + 1,
  R_NUMBER_6 = R_NUMBER_5 + 1,
  R_NUMBER_7 = R_NUMBER_6 + 1,
  R_NUMBER_8 = R_NUMBER_7 + 1,
  R_NUMBER_9 = R_NUMBER_8 + 1,

  W_NUMBER_1 = R_NUMBER_9 + 1,
  W_NUMBER_2 = W_NUMBER_1 + 1,
  W_NUMBER_3 = W_NUMBER_2 + 1,
  W_NUMBER_4 = W_NUMBER_3 + 1,
  W_NUMBER_5 = W_NUMBER_4 + 1,

  COLON = W_NUMBER_5 + 1,
  R_COLON = COLON + 1,

  LAUGH = R_COLON + 1,

  // Ramster
  RAMSTER_RUN_0 = LAUGH + 1,
  RAMSTER_RUN_1 = RAMSTER_RUN_0 + 1,
  RAMSTER_RUN_2 = RAMSTER_RUN_1 + 1,
  RAMSTER_RUN_3 = RAMSTER_RUN_2 + 1,
  RAMSTER_RUN_4 = RAMSTER_RUN_3 + 1,
  RAMSTER_RUN_5 = RAMSTER_RUN_4 + 1,
  RAMSTER_RUN_6 = RAMSTER_RUN_5 + 1,
  RAMSTER_RUN_7 = RAMSTER_RUN_6 + 1,
  RAMSTER_IDLE_0 = RAMSTER_RUN_7 + 1,
  RAMSTER_IDLE_1 = RAMSTER_IDLE_0 + 1,
  RAMSTER_IDLE_2 = RAMSTER_IDLE_1 + 1,
  RAMSTER_IDLE_3 = RAMSTER_IDLE_2 + 1,
  RAMSTER_IDLE_4 = RAMSTER_IDLE_3 + 1,
  RAMSTER_IDLE_5 = RAMSTER_IDLE_4 + 1,
  RAMSTER_GLASS_FRONT = RAMSTER_IDLE_5 + 1,
  RAMSTER_GLASS_BACK = RAMSTER_GLASS_FRONT + 1,
  RAMSTER_GLASS_WALL = RAMSTER_GLASS_BACK + 1,

  // Grapple
  GRAPPLE_POINT = RAMSTER_GLASS_WALL + 1,
  GRAPPLE_OUTLINE = GRAPPLE_POINT + 1,

  // Screen Elements
  TITLE_MENU = GRAPPLE_OUTLINE + 1,
  TITLE_PAUSE = TITLE_MENU + 1,
  TITLE_VICTORY = TITLE_PAUSE + 1,
  TITLE_DEFEAT = TITLE_VICTORY + 1,
  TEXT_MENU = TITLE_DEFEAT + 1,
  TEXT_PAUSE = TEXT_MENU + 1,
  TEXT_GAMEOVER = TEXT_PAUSE + 1,
  LEADERBOARD = TEXT_GAMEOVER + 1,

  BUTTON_LVLUP = LEADERBOARD + 1,
  BUTTON_LVLDOWN = BUTTON_LVLUP + 1,
  BUTTON_START = BUTTON_LVLDOWN + 1,
  BUTTON_RESUME = BUTTON_START + 1,
  BUTTON_RESTART = BUTTON_RESUME + 1,
  BUTTON_MAINMENU = BUTTON_RESTART + 1,
  BUTTON_EXITGAME = BUTTON_MAINMENU + 1,

  BUTTON_LVL1 = BUTTON_EXITGAME + 1,
  BUTTON_LVL2 = BUTTON_LVL1 + 1,
  BUTTON_LVL3 = BUTTON_LVL2 + 1,
  BUTTON_LVL4 = BUTTON_LVL3 + 1,
  BUTTON_LVL5 = BUTTON_LVL4 + 1,
  BUTTON_LVL6 = BUTTON_LVL5 + 1,
  BUTTON_LVL7 = BUTTON_LVL6 + 1,
  BUTTON_LVL8 = BUTTON_LVL7 + 1,
  BUTTON_LVL9 = BUTTON_LVL8 + 1,
  BUTTON_LVL10 = BUTTON_LVL9 + 1,
  BUTTON_LVL11 = BUTTON_LVL10 + 1,
  BUTTON_LVL12 = BUTTON_LVL11 + 1,

  // Screens
  MAIN_MENU_TEXTURE = BUTTON_LVL12 + 1,
  PLAYING_TEXTURE = MAIN_MENU_TEXTURE + 1,
  PAUSE_TEXTURE = PLAYING_TEXTURE + 1,
  END_OF_GAME_TEXTURE = PAUSE_TEXTURE + 1,

  // Legacy invaders
  COMMON_1 = END_OF_GAME_TEXTURE + 1,
  COMMON_2 = COMMON_1 + 1,
  COMMON_3 = COMMON_2 + 1,
  COMMON_4 = COMMON_3 + 1,
  COMMON_5 = COMMON_4 + 1,

  SWARM_1 = COMMON_5 + 1,
  SWARM_2 = SWARM_1 + 1,
  SWARM_3 = SWARM_2 + 1,
  SWARM_4 = SWARM_3 + 1,

  OBSTACLE_1 = SWARM_4 + 1,
  OBSTACLE_2 = OBSTACLE_1 + 1,
  OBSTACLE_3 = OBSTACLE_2 + 1,
  OBSTACLE_4 = OBSTACLE_3 + 1,

  // Levels
  LEVEL_1 = OBSTACLE_4 + 1,
  LEVEL_2 = LEVEL_1 + 1,
  LEVEL_3 = LEVEL_2 + 1,
  LEVEL_4 = LEVEL_3 + 1,
  LEVEL_5 = LEVEL_4 + 1,
  LEVEL_6 = LEVEL_5 + 1,
  LEVEL_TUTORIAL = LEVEL_6 + 1,
  LEVEL_TOWER = LEVEL_TUTORIAL + 1,
  LEVEL_LAB = LEVEL_TOWER + 1,
  LEVEL_UNDER = LEVEL_LAB + 1,
  LEVEL_SNAKE = LEVEL_UNDER + 1,
  LEVEL_TUNNELSMALL = LEVEL_SNAKE + 1,

  // Parallax
  BACKGROUND_0 = LEVEL_TUNNELSMALL + 1,
  BACKGROUND_1 = BACKGROUND_0 + 1,
  BACKGROUND_2 = BACKGROUND_1 + 1,
  BACKGROUND_3 = BACKGROUND_2 + 1,
  BACKGROUND_4 = BACKGROUND_3 + 1,
  BACKGROUND_5 = BACKGROUND_4 + 1,
  BACKGROUND_6 = BACKGROUND_5 + 1,
  BACKGROUND_7 = BACKGROUND_6 + 1,

  // Fireball
  FIREBALL_0 = BACKGROUND_7 + 1,
  FIREBALL_1 = FIREBALL_0 + 1,
  FIREBALL_2 = FIREBALL_1 + 1,
  FIREBALL_3 = FIREBALL_2 + 1,
  FIREBALL_4 = FIREBALL_3 + 1,
  FIREBALL_5 = FIREBALL_4 + 1,
  FIREBALL_6 = FIREBALL_5 + 1,
  FIREBALL_7 = FIREBALL_6 + 1,
  FIREBALL_8 = FIREBALL_7 + 1,
  FIREBALL_9 = FIREBALL_8 + 1,
  FIREBALL_10 = FIREBALL_9 + 1,
  FIREBALL_11 = FIREBALL_10 + 1,

  // Story Slides
  STORYFRAME_INTRO_1 = FIREBALL_11 + 1,
  STORYFRAME_INTRO_2 = STORYFRAME_INTRO_1 + 1,
  STORYFRAME_INTRO_3 = STORYFRAME_INTRO_2 + 1,
  STORYFRAME_INTRO_4 = STORYFRAME_INTRO_3 + 1,
  STORYFRAME_CONCLUSION_1 = STORYFRAME_INTRO_4 + 1,
  STORYFRAME_CONCLUSION_2 = STORYFRAME_CONCLUSION_1 + 1,
  STORYFRAME_CONCLUSION_3 = STORYFRAME_CONCLUSION_2 + 1,

  // confetti frames
  CONFETTI_0 = STORYFRAME_CONCLUSION_3 + 1,
  CONFETTI_1 = CONFETTI_0 + 1,
  CONFETTI_2 = CONFETTI_1 + 1,
  CONFETTI_3 = CONFETTI_2 + 1,
  CONFETTI_4 = CONFETTI_3 + 1,
  CONFETTI_5 = CONFETTI_4 + 1,
  CONFETTI_6 = CONFETTI_5 + 1,
  CONFETTI_7 = CONFETTI_6 + 1,
  CONFETTI_8 = CONFETTI_7 + 1,
  CONFETTI_9 = CONFETTI_8 + 1,
  CONFETTI_10 = CONFETTI_9 + 1,
  CONFETTI_11 = CONFETTI_10 + 1,
  CONFETTI_12 = CONFETTI_11 + 1,
  CONFETTI_13 = CONFETTI_12 + 1,
  CONFETTI_14 = CONFETTI_13 + 1,
  CONFETTI_15 = CONFETTI_14 + 1,
  CONFETTI_16 = CONFETTI_15 + 1,
  CONFETTI_17 = CONFETTI_16 + 1,
  CONFETTI_18 = CONFETTI_17 + 1,
  CONFETTI_19 = CONFETTI_18 + 1,
  CONFETTI_20 = CONFETTI_19 + 1,
  CONFETTI_21 = CONFETTI_20 + 1,
  CONFETTI_22 = CONFETTI_21 + 1,
  CONFETTI_23 = CONFETTI_22 + 1,
  CONFETTI_24 = CONFETTI_23 + 1,
  CONFETTI_25 = CONFETTI_24 + 1,
  CONFETTI_26 = CONFETTI_25 + 1,
  CONFETTI_27 = CONFETTI_26 + 1,
  CONFETTI_28 = CONFETTI_27 + 1,
  CONFETTI_29 = CONFETTI_28 + 1,
  CONFETTI_30 = CONFETTI_29 + 1,
  CONFETTI_31 = CONFETTI_30 + 1,
  CONFETTI_32 = CONFETTI_31 + 1,
  CONFETTI_33 = CONFETTI_32 + 1,
  CONFETTI_34 = CONFETTI_33 + 1,
  CONFETTI_35 = CONFETTI_34 + 1,
  CONFETTI_36 = CONFETTI_35 + 1,
  CONFETTI_37 = CONFETTI_36 + 1,
  CONFETTI_38 = CONFETTI_37 + 1,
  CONFETTI_39 = CONFETTI_38 + 1,
  CONFETTI_40 = CONFETTI_39 + 1,
  CONFETTI_41 = CONFETTI_40 + 1,
  CONFETTI_42 = CONFETTI_41 + 1,
  CONFETTI_43 = CONFETTI_42 + 1,
  CONFETTI_44 = CONFETTI_43 + 1,
  CONFETTI_45 = CONFETTI_44 + 1,
  CONFETTI_46 = CONFETTI_45 + 1,
  CONFETTI_47 = CONFETTI_46 + 1,
  CONFETTI_48 = CONFETTI_47 + 1,
  CONFETTI_49 = CONFETTI_48 + 1,
  CONFETTI_50 = CONFETTI_49 + 1,
  CONFETTI_51 = CONFETTI_50 + 1,
  CONFETTI_52 = CONFETTI_51 + 1,
  CONFETTI_53 = CONFETTI_52 + 1,
  CONFETTI_54 = CONFETTI_53 + 1,
  CONFETTI_55 = CONFETTI_54 + 1,
  CONFETTI_56 = CONFETTI_55 + 1,
  CONFETTI_57 = CONFETTI_56 + 1,
  CONFETTI_58 = CONFETTI_57 + 1,

  TEXTURE_COUNT = CONFETTI_58 + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class MUSIC
{
  MENU = 0,
  OBLANKA = MENU + 1,
  PARADRIZZLE = OBLANKA + 1,
  WINDCATCHER = PARADRIZZLE + 1,
  PROMENADE = WINDCATCHER + 1,
  SPABA = PROMENADE + 1,
  COTTONPLANES = SPABA + 1,
  PENCILCRAYONS = COTTONPLANES + 1,
  MOONTOWNSHORES = PENCILCRAYONS + 1,
  MUSIC_COUNT = MOONTOWNSHORES + 1,
};
const int music_count = (int)MUSIC::MUSIC_COUNT;

enum class FX
{
  FX_DESTROY_ENEMY = 0,
  FX_DESTROY_ENEMY_FAIL = FX_DESTROY_ENEMY + 1,
  FX_JUMP = FX_DESTROY_ENEMY_FAIL + 1,
  FX_GRAPPLE = FX_JUMP + 1,

  FX_BALL_ROLLING = FX_GRAPPLE + 1,
  FX_BALL_FLAMMING = FX_BALL_ROLLING + 1,

  FX_COUNT = FX_BALL_FLAMMING + 1,
};
const int fx_count = (int)FX::FX_COUNT;

enum class EFFECT_ASSET_ID
{
  LEGACY_EGG = 0,
  LEGACY_CHICKEN = LEGACY_EGG + 1,
  TEXTURED = LEGACY_CHICKEN + 1,
  VIGNETTE = TEXTURED + 1,
  PARALLAX = VIGNETTE + 1,
  TRANSLUCENT = PARALLAX + 1,
  FIREBALL = TRANSLUCENT + 1,
  RAMSTER = FIREBALL + 1,
  EFFECT_COUNT = RAMSTER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
  LEGACY_CHICKEN = 0,
  SPRITE = LEGACY_CHICKEN + 1,
  LEGACY_EGG = SPRITE + 1,
  DEBUG_LINE = LEGACY_EGG + 1,
  SCREEN_TRIANGLE = DEBUG_LINE + 1,
  GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct Animation
{
  std::vector<TEXTURE_ASSET_ID> frames; // list of textures for animation
  float frame_time;                     // time per frame in milliseconds
  float elapsed_time;                   // tracks elapsed time
  int current_frame;                    // current frame index

  Animation(std::vector<TEXTURE_ASSET_ID> frames, float frame_time)
      : frames(frames), frame_time(frame_time), elapsed_time(0), current_frame(0)
  {
  }
};

struct RenderRequest
{
  TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
  EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
  GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

  // embed optional animation data
  std::vector<TEXTURE_ASSET_ID> animation_frames; // animation frames
  std::vector<float> animation_frames_scale;      // optionally assign scale to each frame independently
  bool is_loop = true;                            // if true, loop the animation
  bool is_visible = true;                         // if false, do not render this entity
  float animation_frame_time = 0;                 // time per frame in ms
  float animation_elapsed_time = 0;               // relative elapsed time
  int animation_current_frame = 0;                // current frame index
};

struct FireBall
{
};

struct PhysicsBody
{
  b2BodyId bodyId;
};

struct GoalZone
{
  vec2 bl_boundary;
  vec2 tr_boundary;
  bool hasTriggered;
};

struct Grapple
{
  b2JointId jointId;
  b2BodyId ballBodyId;
  b2BodyId grappleBodyId;
  Entity lineEntity;
};

struct GrapplePoint
{
  vec2 position;
  bool active;
  b2BodyId bodyId;
};

struct Camera
{
  vec2 position;     // Camera's world position
  float zoom = 1.0f; // Optional zoom factor
};

struct PlayerPhysics
{
  bool isGrounded;
};

struct Line
{
  vec2 start_pos = {0, 0};
  vec2 end_pos = {10, 10}; // default to diagonal line
};

struct EnemyPhysics
{
  bool isGrounded;
};

struct LevelLayer
{
};

struct BackgroundLayer
{
};

struct PlayerRotatableLayer
{
};

struct PlayerNonRotatableLayer
{
};

struct PlayerTopLayer
{
};

struct PlayerMidLayer
{
};

struct PlayerBottomLayer
{
};

struct IdleAnimation
{
};

struct RunAnimation
{
};

struct HealthBar
{
  float health;
};

struct Score
{
  int score;
  Entity digits[4];
};

struct Timer
{
  Entity digits[7];
};

struct UI
{
};

struct LBTimer
{
  Entity digits[10];
};