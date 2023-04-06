#pragma once

#include <functional>

#include "Card.h"

class PlayerState;

using QuestReqs = std::function<int(const PlayerState&)>;
using CardPred = std::function<bool(const PlayerState&, const Card*)>;
struct Quest {
	int idx = -1;
	Resource imaginaryPrice;
	QuestReqs cardsLeft;
	CardPred isCardForQuest;
	CardEffect effect;
	ImaginaryCardEffect imaginaryEffect;
	bool isBasic = false;
};

