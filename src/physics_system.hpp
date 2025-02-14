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
	PhysicsSystem(b2World& world);
	~PhysicsSystem();

private:
	// box2d world instance (shared between systems)
	b2World& world;
};