#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <array>
#include <map>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

// box2D
#include <box2d/box2d.h>

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};


/* All screens that we'll be using in our game.
const extern enum SCREENS {

    // Level selector 
    // From: Launching Game, Pause - H, End of Game - ESC
    // To: Playing - Any level selection + ENTER, Exit - ESC
    MAIN_MENU = 0,

    // Gameplay here
    // From: Main menu - any level selection + ENTER, Pause - ESC
    // To: Pause - ESC
    PLAYING = MAIN_MENU + 1,

    // Pauses gameplay, lets player resume, return to main menu, or keep playing
    // From: Playing - ESC
    // To: Main menu - H, Playing - ESC, (reset) Playing - R
    PAUSE = PLAYING + 1,

    // When player finishes the level or dies
    // From: Playing - auto-triggers on win or death.
    // To: Main Menu - ESC, (reset) Playing - R, (next level) Playing - ENTER
    END_OF_GAME = PAUSE + 1
};
*/

//
// level constants
// TODO: if we allow levels of varying sizes, this needs to be updated dynamically between levels
//
extern int WORLD_WIDTH_TILES;
extern int WORLD_HEIGHT_TILES;

extern int WORLD_WIDTH_PX;
extern int WORLD_HEIGHT_PX;

//
// game constants
//
const int WINDOW_WIDTH_PX = 1366;
const int WINDOW_HEIGHT_PX = 768;

const float VIEWPORT_WIDTH_PX = WINDOW_WIDTH_PX;
const float VIEWPORT_HEIGHT_PX = WINDOW_HEIGHT_PX;
const float ASPECT_RATIO = VIEWPORT_WIDTH_PX / VIEWPORT_HEIGHT_PX;

const int TILE_WIDTH = 128;
const int TILE_HEIGHT = 128;

const int GRID_CELL_WIDTH_PX = 128;
const int GRID_CELL_HEIGHT_PX = 128;
const int GRID_LINE_WIDTH_PX = 1;

const int TOWER_TIMER_MS = 1000;
const int MAX_TOWERS_START = 5;

const int ENEMY_SPAWN_RATE_MS = 15 * 1000;

const int INVADER_VELOCITY_GREY = 80;
const int INVADER_VELOCITY_RED = 70;
const int INVADER_VELOCITY_GREEN = 50;
const int INVADER_VELOCITY_BLUE = 30;

const int INVADER_HEALTH_GREY = 20;
const int INVADER_HEALTH_RED = 30;
const int INVADER_HEALTH_GREEN = 50;
const int INVADER_HEALTH_BLUE = 120;

const int PROJECTILE_VELOCITY = -100;
const int PROJECTILE_DAMAGE = 10;

// Amount of time to stop an enemy after colliding (if player loses collision)
const float ENEMY_FREEZE_TIME_MS = 1500;
// Delay before showing end of game screen after game is over
const int TIMER_GAME_END = 1500;
// Minimum amount of speed the player needs after a collision to "win". Tune down for easier gameplay and vice versa.
const float MIN_COLLISION_SPEED = 500;

// Amount of time before refreshing FPS counter. This will eliminate window flickering from too many updates per second.
const int FPS_UPDATE_COOLDOWN_MS = 250;
// Granularity in ms of time
const int TIME_GRANULARITY = 1000;

// enemy types that we will be supporting.
const enum ENEMY_TYPES {
    SWARM = 1,
    COMMON = SWARM + 1,
    OBSTACLE = COMMON + 1
};

// KEY STATES
const std::vector<int> PLAYER_CONTROL_KEYS = {
    GLFW_KEY_W,
    GLFW_KEY_A,
    GLFW_KEY_S,
    GLFW_KEY_D,
    GLFW_KEY_SPACE
};

// WORLD PHYSICS
const float GRAVITY = -980; // cm/s� (centimeters per second squared)

// PLAYER 2DBODY
// PLAYER PHYSICS
const float BALL_INITIAL_POSITION_X = 100.0;
const float BALL_INITIAL_POSITION_Y = 800.0;

// Player input related physics
const float BALL_GROUNDED_MOVEMENT_FORCE = 25000.0f; // kg�cm/s� (dynes)
const float BALL_AIR_STRAFE_FORCE_MULTIPLIER = 0.5f;
const float BALL_JUMP_IMPULSE = 8000.0f; // kg�cm/s (dynes�s)

// A ball of radius 32cm has area ~3200cm�.
// We should pick a value that yields a reasonable weight-to-area ratio like a density of 0.01.
// Thus our 32cm ball would have a weight of only 32kg.
const float BALL_RADIUS = 32.0;
const float BALL_DENSTIY = 0.01f; // kg/cm� (kilograms per square centimeter)
const float BALL_FRICTION = 0.1f;
const float BALL_RESTITUTION = 0.0f;
const float BALL_ANGULAR_DAMPING = 0.75f; // 1/s (inverse seconds)

// HP that the player starts with
const float PLAYER_STARTING_HP = 5;

// ENEMY 2DBODY
// Shares most of player 2D body but different density, friction, restitution, etc.
const float ENEMY_GROUNDED_MOVEMENT_FORCE = 1875.0f; // kg�cm/s� (dynes)
const float ENEMY_JUMP_IMPULSE = 2000.0f; // kg�cm/s (dynes�s)

const float ENEMY_RADIUS = 25.0;
const float ENEMY_DENSITY = 0.00125f; // kg/cm� (kilograms per square centimeter); lower number = less speed lost on collision, less enemy momentum.
const float ENEMY_FRICTION = 0.1f; //enemy friction. for now we're setting it low so it's less affected by contact with floor slowing it down.
const float ENEMY_RESTITUTION = 0.5f; //enemy bounciness... increase this number to make things more chaotic.

// SWARM ENEMY PROXIMITY - MAX DELTA X OR DELTA Y FROM SWARM BEFORE REJOINING
const float SWARM_ENEMY_PROXIMITY = 1.5 * GRID_CELL_WIDTH_PX;

// TERRAIN PHYSICS
const float TERRAIN_DEFAULT_FRICTION = 0.2f;
const float TERRAIN_DEFAULT_RESTITUTION = 0.0f;
const float CURVED_RAMP_FRICTION = 0.01f;
const float CURVED_RAMP_RESTITUTION = 0.01f;
const float WALL_DEFAULT_THICKNESS = 4.0f;

//GRAPPLE PHYSICS
const float GRAPPLE_DETRACT_GROUNDED =  20.0f;
const float GRAPPLE_DETRACT_W = 5.0f;
const float GRAPPLE_HERTZ_GROUNDED = 1.0f;
const float GRAPPLE_DAMPING_GROUNDED = 0.5f;
const float GRAPPLE_MAX_LENGTH = 450.0f; //450.0f;
const float GRAPPLE_MIN_LENGTH = 100.0f;

// change this to change the clickable area to attach to a grapple point
const float GRAPPLE_ATTACH_ZONE_RADIUS = 128.0f; //256.0f;

// These are hard coded to the dimensions of the entity's texture
// invaders are 64x64 px, but cells are 60x60
const float INVADER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float INVADER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// explosions are 64x64 px, but cells are 60x60
const float EXPLOSION_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float EXPLOSION_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// towers are 64x64 px, but cells are 60x60
const float TOWER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float TOWER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// projectiles are 64x64 px, but cells are 60x60
const float PROJECTILE_BB_WIDTH = (float)GRID_CELL_WIDTH_PX*0.5f;
const float PROJECTILE_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX*0.5f;

// Level loading
const int TILED_TO_GRID_PIXEL_SCALE = 1;

const std::string LEVEL_DIR_FILEPATH = "../levels/";
const std::string JSON_POLYLINE_ATTR = "polyline";
const std::string JSON_POLYGON_ATTR = "polygon";
const std::string JSON_BALL_SPAWNPOINT = "ball_spawnpoint";
const std::string JSON_SWARM_SPAWNPOINT = "swarm_spawnpoint";


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};


// rotate enemies_killed
inline glm::vec2 rotateAroundPoint(const vec2& point, const vec2& origin, float angleRadians) {
    // Translate point to origin
    float translatedX = point.x - origin.x;
    float translatedY = point.y - origin.y;

    // Apply rotation
    float cosTheta = std::cos(angleRadians);
    float sinTheta = std::sin(angleRadians);
    float rotatedX = translatedX * cosTheta - translatedY * sinTheta;
    float rotatedY = translatedX * sinTheta + translatedY * cosTheta;

    // Translate back
    vec2 result;
    result.x = rotatedX + origin.x;
    result.y = rotatedY + origin.y;
    return result;
}

bool gl_has_errors();

// used for delimiting point names
inline std::vector<std::string> split(std::string s, std::string delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}
