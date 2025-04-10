#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface *> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<CurrentScreen> currentScreen;
	ComponentContainer<ScreenElement> screenElements;
	ComponentContainer<UIButton> buttons;
	ComponentContainer<Level> levels;
	ComponentContainer<StoryFrame> storyFrames;
	ComponentContainer<Screen> screens; // legacy code. remove support after finishing screen element
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Mesh *> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<GridLine> gridLines;
	ComponentContainer<PhysicsBody> physicsBodies;
	ComponentContainer<PlayerPhysics> playerPhysics;
	ComponentContainer<EnemyPhysics> enemyPhysics;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Line> lines;
	ComponentContainer<Grapple> grapples;
	ComponentContainer<GrapplePoint> grapplePoints;
	ComponentContainer<LevelLayer> levelLayers;
	ComponentContainer<BackgroundLayer> backgroundLayers;
	ComponentContainer<PlayerRotatableLayer> playerRotatableLayers;
	ComponentContainer<PlayerNonRotatableLayer> playerNonRotatableLayers;
	ComponentContainer<PlayerTopLayer> playerTopLayer;
	ComponentContainer<PlayerMidLayer> playerMidLayer;
	ComponentContainer<PlayerBottomLayer> playerBottomLayer;
	ComponentContainer<GoalZone> goalZones;
	ComponentContainer<FireBall> fireballs;
	ComponentContainer<RunAnimation> runAnimations;
	ComponentContainer<IdleAnimation> idleAnimations;
	ComponentContainer<HealthBar> healthbars;
	ComponentContainer<Score> scores;
	ComponentContainer<Timer> timers;
	ComponentContainer<UI> uis;
	ComponentContainer<LBTimer> lbtimers;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		registry_list.push_back(&currentScreen);
		registry_list.push_back(&screenElements);
		registry_list.push_back(&buttons);
		registry_list.push_back(&levels);
		registry_list.push_back(&storyFrames);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&physicsBodies);
		registry_list.push_back(&playerPhysics);
		registry_list.push_back(&lines);
		registry_list.push_back(&enemyPhysics);
		registry_list.push_back(&grapples);
		registry_list.push_back(&grapplePoints);
		registry_list.push_back(&levelLayers);
		registry_list.push_back(&cameras);
		registry_list.push_back(&goalZones);
		registry_list.push_back(&backgroundLayers);
		registry_list.push_back(&fireballs);
		registry_list.push_back(&healthbars);
		registry_list.push_back(&scores);
		registry_list.push_back(&timers);
		registry_list.push_back(&uis);
		registry_list.push_back(&lbtimers);
	}

	void clear_all_components()
	{
		for (ContainerInterface *reg : registry_list)
			reg->clear();
	}

	void list_all_components()
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface *reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e)
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface *reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e)
	{
		for (ContainerInterface *reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;