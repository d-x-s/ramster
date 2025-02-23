#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <array>

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

//
// game constants
//
const int WINDOW_WIDTH_PX = 840;
const int WINDOW_HEIGHT_PX = 600;

const int GRID_CELL_WIDTH_PX = 60;
const int GRID_CELL_HEIGHT_PX = 60;
const int GRID_LINE_WIDTH_PX = 2;

const int TOWER_TIMER_MS = 1000;	// number of milliseconds between tower shots
const int MAX_TOWERS_START = 5;

const int ENEMY_SPAWN_RATE_MS = 3 * 1000;

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

// KEY STATES
const std::vector<int> PLAYER_CONTROL_KEYS = {
    GLFW_KEY_W,
    GLFW_KEY_A,
    GLFW_KEY_S,
    GLFW_KEY_D,
    GLFW_KEY_SPACE
};

// PLAYER 2DBODY
// PLAYER PHYSICS
const float GRAVITY = -980; // should be implemented as a force, not an impulse.

const float GROUNDED_MOVEMENT_FORCE = 300.0f;
const float AIR_STRAFE_FORCE_MULTIPLIER = 0.5f;
const float JUMP_IMPULSE = 80.0f;

// WARNING: don't mess with the density, if you do all the forces have to be re-tuned.
const float BALL_DENSTIY = 0.50f;
const float BALL_FRICTION = 0.01f;
const float BALL_RESTITUTION = 0.3f;
const float BALL_ANGULAR_DAMPING = 75.0f;

// ENEMY 2DBODY
// Shares most of player 2D body but different density, friction, restitution, etc.
const float ENEMY_DENSITY = 0.5f; // lower number = less affected by gravity 
const float ENEMY_FRICTION = 0.02f; //enemy friction. for now we're setting it low so it's less affected by gravity & spins less
const float ENEMY_RESTITUTION = 0.4f; //enemy bounciness... increase this number to make things more chaotic.

// TERRAIN PHYSICS
const float TERRAIN_DEFAULT_FRICTION = 0.2f;
const float TERRAIN_DEFAULT_RESTITUTION = 0.5f;

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

bool gl_has_errors();
