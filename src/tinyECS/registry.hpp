#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<CurrentScreen> currentScreen;
	ComponentContainer<ScreenElement> screenElements;
	ComponentContainer<Button> buttons;
	ComponentContainer<Screen> screens; // legacy code. remove support after finishing screen element
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	// IMPORTANT: Add any new CC's below to the registry_list
	ComponentContainer<Tower> towers;
	ComponentContainer<GridLine> gridLines;
	ComponentContainer<Invader> invaders;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Explosion> explosions;
	ComponentContainer<PhysicsBody> physicsBodies;
	ComponentContainer<PlayerPhysics> playerPhysics;
	ComponentContainer<EnemyPhysics> enemyPhysics;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Line> lines;
	ComponentContainer<Grapple> grapples;
	ComponentContainer<GrapplePoint> grapplePoints;
	ComponentContainer<TutorialTile> tutorialTiles;
	ComponentContainer<LevelLayer> levelLayers;
	ComponentContainer<BackgroundLayer> backgroundLayers;
	ComponentContainer<GoalZone> goalZones;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&currentScreen);
		registry_list.push_back(&screenElements);
		registry_list.push_back(&buttons);
		registry_list.push_back(&screens); // remove after implementation of screenElement/Button
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&towers);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&invaders);
		registry_list.push_back(&projectiles);

		registry_list.push_back(&physicsBodies);
		registry_list.push_back(&playerPhysics);
		registry_list.push_back(&lines);
		registry_list.push_back(&enemyPhysics);
		registry_list.push_back(&grapples);
		registry_list.push_back(&grapplePoints);

		registry_list.push_back(&levelLayers);
		registry_list.push_back(&cameras);

		registry_list.push_back(&goalZones);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;