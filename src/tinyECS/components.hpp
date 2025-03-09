#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{

};

// Enemy component
struct Enemy
{

};

// Tower
struct Tower {
	float range;	// for vision / detection
	int timer_ms;	// when to shoot - this could also be a separate timer component...
};

// Invader
struct Invader {
	int health;
};

// Projectile
struct Projectile {
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
struct Motion {
	vec2  position = { 0, 0 }; // pixel coordinates
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
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
struct GridLine {
	vec2 start_pos = {  0,  0 };
	vec2 end_pos   = { 10, 10 };	// default to diagonal line
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
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
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

enum class TEXTURE_ASSET_ID {
	BLUE_INVADER_1 = 0,
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
	RAMSTER_1 = TOWER + 1,
	GRAPPLE_POINT = RAMSTER_1 + 1,
	EXPLOSION_1 = GRAPPLE_POINT + 1,
	EXPLOSION_2 = EXPLOSION_1 + 1,
	EXPLOSION_3 = EXPLOSION_2 + 1,
	FLOATER_1 = EXPLOSION_3 + 1,
	FLOATER_2 = FLOATER_1 + 1,
	FLOATER_3 = FLOATER_2 + 1,
	TEXTURE_COUNT = FLOATER_3 + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	VIGNETTE = TEXTURED + 1,
	EFFECT_COUNT = VIGNETTE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct Animation {
	std::vector<TEXTURE_ASSET_ID> frames; // list of textures for animation
	float frame_time; // time per frame in milliseconds
	float elapsed_time; // tracks elapsed time
	int current_frame; // current frame index

	Animation(std::vector<TEXTURE_ASSET_ID> frames, float frame_time)
		: frames(frames), frame_time(frame_time), elapsed_time(0), current_frame(0) {
	}
};

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

	// embed optional animation data
	std::vector<TEXTURE_ASSET_ID> animation_frames;	// animation frames
	std::vector<float> animation_frames_scale;		  // optionally assign scale to each frame independently
	bool is_loop = true;							              // if true, loop the animation
	float animation_frame_time = 0;					        // time per frame in ms
	float animation_elapsed_time = 0;				        // relative elapsed time
	int animation_current_frame = 0;				        // current frame index
};

struct Explosion { };

struct PhysicsBody {
	b2BodyId bodyId;
	b2ShapeId shapeId;
};

struct GrapplePoint {

};

struct Grapple {
	b2JointId jointId;
	b2BodyId ballBodyId;
	b2BodyId grappleBodyId;
	Entity lineEntity;
};

struct Camera {
	vec2 position;  // Camera's world position
	float zoom = 1.0f;  // Optional zoom factor
};

struct PlayerPhysics {
	bool isGrounded;
};

struct Line {
	vec2 start_pos = { 0,  0 };
	vec2 end_pos = { 10, 10 };	// default to diagonal line
};

struct EnemyPhysics {
	bool isGrounded;
};
