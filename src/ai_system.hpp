#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "iostream"

class AISystem
{
public:
	void step(float elapsed_ms);

private:
	bool tooCloseToSwarm(Entity swarmEnemy, vec2& enemyToAvoid);

	bool tooFarFromSwarm(Entity swarmEnemy, vec2& closestSwarmEntity);
};