#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

#include <box2d/box2d.h>

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);
	explicit PhysicsSystem(b2WorldId worldId);
	~PhysicsSystem();
	void updateGrappleLines();
	void update_fireball();
	void update_player_animation();
	void updateHealthBar(vec2 camPos);
	void updateScore(vec2 camPos);
	void updateTimer(vec2 camPos);

private:
	b2WorldId worldId;
};