#pragma once

#include <vector>
#include <array>
#include <random>
#include <string>

#include "Quest.h"
#include "Resource.h"
#include "PlayerState.h"

struct FieldAction {
	size_t idx = -1;
	Resource price{};
	Resource profit{};
	int slots = 1;
	std::vector<int> deployedWorkers;
};

std::vector<FieldAction> genBaseActions();
std::vector<FieldAction> genForestActions(std::default_random_engine& rng);
std::vector<Quest> genAllQuests(std::default_random_engine& rng);

struct GameState
{
	std::vector<FieldAction> baseActions = genBaseActions();
	std::vector<FieldAction> forestActions;

	std::vector<PlayerState> players;
	std::vector<const Card*> graveyard;
	std::vector<const Card*> deck;
	std::vector<const Card*> meadow;
	std::vector<Quest> quests;
};

int countInCityByName(const PlayerState& ps, const std::string& name);
