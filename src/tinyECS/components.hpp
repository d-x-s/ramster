#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"


// Any element on the screen. Title, Label, Button, Etc. This is what gets rendered.
struct ScreenElement {

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
struct Button {

    // Identifies the button
    std::string function;

};

// Indicates that a button has a level
struct Level {
	int level;
};

// Indicates that this screen element is part of a story frame
struct StoryFrame {
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
struct CurrentScreen {
    std::string current_screen = "MAIN MENU"; // start on main menu
};

// Player component
struct Player
{
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

// Tower
struct Tower
{
  float range;  // for vision / detection
  int timer_ms; // when to shoot - this could also be a separate timer component...
};

// Invader
struct Invader
{
  int health;
};

// Projectile
struct Projectile
{
  int damage;
};

// used for Entities that cause damage
struct Deadly
{
};

// used for edible entities
struct Eatable
{
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
	Collision(Entity& other) { this->other = other; };
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
	// Ramster 
	RAMSTER_DEBUG = 0,
	RAMSTER_GLASS_FRONT = RAMSTER_DEBUG + 1,
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

	BUTTON_LVLUP = TEXT_GAMEOVER + 1,
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
	BLUE_INVADER_1 = END_OF_GAME_TEXTURE + 1,
	BLUE_INVADER_2 = BLUE_INVADER_1 + 1,
	BLUE_INVADER_3 = BLUE_INVADER_2 + 1,
	RED_INVADER_1 = BLUE_INVADER_3 + 1,
	RED_INVADER_2 = RED_INVADER_1 + 1,
	RED_INVADER_3 = RED_INVADER_2 + 1,
	GREEN_INVADER_1 = RED_INVADER_3 + 1,
	GREEN_INVADER_2 = GREEN_INVADER_1 + 1,
	GREEN_INVADER_3 = GREEN_INVADER_2 + 1,
	GREY_INVADER_1 = GREEN_INVADER_3 + 1,
	GREY_INVADER_2 = GREY_INVADER_1 + 1,
	GREY_INVADER_3 = GREY_INVADER_2 + 1,
	TOWER = GREY_INVADER_3 + 1,
	EXPLOSION_1 = TOWER + 1,
	EXPLOSION_2 = EXPLOSION_1 + 1,
	EXPLOSION_3 = EXPLOSION_2 + 1,
	FLOATER_1 = EXPLOSION_3 + 1,
	FLOATER_2 = FLOATER_1 + 1,
	FLOATER_3 = FLOATER_2 + 1,

	COMMON_1 = FLOATER_3 + 1,
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

	// Tiles
	HALF_RAMP_BL = OBSTACLE_4 + 1,
	HALF_RAMP_BR = HALF_RAMP_BL + 1,
	HALF_RAMP_TL = HALF_RAMP_BR + 1,
	HALF_RAMP_TR = HALF_RAMP_TL + 1,
	HALF_SQUARE_2_BOTTOM = HALF_RAMP_TR + 1,
	HALF_SQUARE_2_LEFT = HALF_SQUARE_2_BOTTOM + 1,
	HALF_SQUARE_2_RIGHT = HALF_SQUARE_2_LEFT + 1,
	HALF_SQUARE_2_TOP = HALF_SQUARE_2_RIGHT + 1,
	HALF_SQUARE_BOTTOM = HALF_SQUARE_2_TOP + 1,
	HALF_SQUARE_LEFT = HALF_SQUARE_BOTTOM + 1,
	HALF_SQUARE_RIGHT = HALF_SQUARE_LEFT + 1,
	HALF_SQUARE_TOP = HALF_SQUARE_RIGHT + 1,
	SMOOTH_RAMP_BL = HALF_SQUARE_TOP + 1,
	SMOOTH_RAMP_BR = SMOOTH_RAMP_BL + 1,
	SMOOTH_RAMP_TL = SMOOTH_RAMP_BR + 1,
	SMOOTH_RAMP_TR = SMOOTH_RAMP_TL + 1,
	SQUARE_TILE_1 = SMOOTH_RAMP_TR + 1,
	SQUARE_TILE_2 = SQUARE_TILE_1 + 1,
	TALL_RAMP_BL = SQUARE_TILE_2 + 1,
	TALL_RAMP_BR = TALL_RAMP_BL + 1,
	TALL_RAMP_TL = TALL_RAMP_BR + 1,
	TALL_RAMP_TR = TALL_RAMP_TL + 1,
	TESLA_TRAP_1_BOTTOM = TALL_RAMP_TR + 1,
	TESLA_TRAP_1_LEFT = TESLA_TRAP_1_BOTTOM + 1,
	TESLA_TRAP_1_RIGHT = TESLA_TRAP_1_LEFT + 1,
	TESLA_TRAP_1_TOP = TESLA_TRAP_1_RIGHT + 1,
    
	// Tutorial
	TUTORIAL_SPACEBAR = TESLA_TRAP_1_TOP + 1,
	TUTORIAL_MOVE = TUTORIAL_SPACEBAR + 1,
	TUTORIAL_GRAPPLE = TUTORIAL_MOVE + 1,
	TUTORIAL_DESTROY = TUTORIAL_GRAPPLE + 1,

	// Levels
	LEVEL_1 = TUTORIAL_DESTROY + 1,
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
	BACKGROUND = LEVEL_TUNNELSMALL + 1,

	// Fireball
	FIREBALL_0 = BACKGROUND + 1,
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

	// Story frames
	STORYFRAME_INTRO_1 = FIREBALL_11 + 1,
	STORYFRAME_INTRO_2 = STORYFRAME_INTRO_1 + 1,
	STORYFRAME_INTRO_3 = STORYFRAME_INTRO_2 + 1,
	STORYFRAME_INTRO_4 = STORYFRAME_INTRO_3 + 1,
	STORYFRAME_CONCLUSION_1 = STORYFRAME_INTRO_4 + 1,
	STORYFRAME_CONCLUSION_2 = STORYFRAME_CONCLUSION_1 + 1,
	STORYFRAME_CONCLUSION_3 = STORYFRAME_CONCLUSION_2 + 1,
	STORYFRAME_CONCLUSION_4 = STORYFRAME_CONCLUSION_3 + 1,

  TEXTURE_COUNT = STORYFRAME_CONCLUSION_4 + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class MUSIC
{
  MENU = 0,
  OBLANKA = MENU + 1,
  PARADRIZZLE = OBLANKA + 1,
  WINDCATCHER = PARADRIZZLE + 1,
  PROMENADE = WINDCATCHER + 1,

  STORY_INTRO = PROMENADE + 1,
  STORY_CONCLUSION = STORY_INTRO + 1,

  MUSIC_COUNT = STORY_CONCLUSION + 1,
};
const int music_count = (int)MUSIC::MUSIC_COUNT;

enum class FX
{
  FX_DESTROY_ENEMY = 0,
  FX_DESTROY_ENEMY_FAIL = FX_DESTROY_ENEMY + 1,
  FX_JUMP = FX_DESTROY_ENEMY_FAIL + 1,
  FX_GRAPPLE = FX_JUMP + 1,
  
  FX_COUNT = FX_GRAPPLE + 1,
};
const int fx_count = (int)FX::FX_COUNT;

enum class EFFECT_ASSET_ID
{
  COLOURED = 0,
  EGG = COLOURED + 1,
  CHICKEN = EGG + 1,
  TEXTURED = CHICKEN + 1,
  VIGNETTE = TEXTURED + 1,
  PARALLAX = VIGNETTE + 1,
  TRANSLUCENT = PARALLAX + 1,
  FIREBALL = TRANSLUCENT + 1,
  EFFECT_COUNT = FIREBALL + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
  CHICKEN = 0,
  SPRITE = CHICKEN + 1,
  EGG = SPRITE + 1,
  DEBUG_LINE = EGG + 1,
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

struct Explosion
{
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

// TODO remove this?
struct Grapple
{
  b2JointId jointId;
  b2BodyId ballBodyId;
  b2BodyId grappleBodyId;
  Entity lineEntity;
};

struct GrapplePoint {
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

struct TutorialTile
{

};

struct LevelLayer
{

};

struct BackgroundLayer
{

};

struct PlayerVisualLayer
{

};
