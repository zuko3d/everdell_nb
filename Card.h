#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>

#include "CardEffect.h"
#include "Primitives.h"
#include "Resource.h"

struct Card
{
	std::string name;
	std::string mainBuildingName;
	const Card* mainBuildingCard = nullptr;
	const Card* residentCard = nullptr;
	std::function<bool(const Card*)> canHaveResident = [](const Card* card) { return false; };
	std::function<bool(const Card*)> canBeResident = [](const Card* card) { return false; };
	const Card* synergyBase = nullptr;
	std::vector<const Card*> synergyDuo;
	std::function<int(const Card*)> purpleValue;
	std::string attachableToName = "";
	const Card* attachableToCard = nullptr;
	Color color;
	int greenProcOrder = -1;
	bool deployable = false;
	int oppCanJoinForPoints = 0;
	int winPoints;
	int occupiedCitySlots = 1;
	bool isBuilding;
	bool isUnique;
	Resource cost;
	size_t amountInGame;

	CardEffect effect = [](GameEngine&) { throw std::logic_error("No cast effect!"); };
	CardEffect deployEffect = [](GameEngine&) { throw std::logic_error("No deploy effect!"); };
	ImaginaryCardEffect imagineProcEffect;
	ImaginaryCardEffect imagineDeployEffect;
	ImaginaryCardEffect imagineSynergyBonus;
	ImaginaryLaterEffect imagineLaterEffect;
	
	std::string tag = "";

	int idx = -1;

	static std::vector<Card> genAllCards();

	static const Card* husbandCard;
	static const Card* wifeCard;
	static const Card* traktirCard;
	static const Card* bardCard;
	static const Card* hightowerCard;
	static const Card* zemlekopCard;
	static const Card* courtCard;
	static const Card* judgeCard;
	static const Card* healerCard;
	static const Card* pigeonCard;
	static const Card* rangerCard;
	static const Card* dvornikCard;
	static const Card* towerCard;
	static const Card* historicCard;
	static const Card* razvaliniCard;
	static const Card* lavochnikCard;
	static const Card* postCard;
	static const Card* graveyardCard;
	static const Card* shepherdCard;
	static const Card* monkCard;
	static const Card* dungeonCard;
	static const Card* rezchikCard;
	static const Card* chapelCard;
	static const Card* grobovshikCard;
	static const Card* vetochnikCard;
	static const Card* teacherCard;
	static const Card* univerCard;
	static const Card* queenCard;
	static const Card* craneCard;
	static const Card* inventorCard;
	static const Card* traktirshikCard;
	static const Card* skladCard;
	static const Card* farmCard;
	static const Card* jokerCard;

	static std::unordered_map<std::string, const Card*> cardByName;
	static std::vector<Card> allCardsInGame;
	static std::array<CardEffect, TriggerType::NoneTrigger> allTriggers;
};

