#pragma once

#include "PlayerState.h"
#include "Card.h"

#include <iomanip>
#include <sstream>
#include <random>

class GameEngine;

struct BotParams {
	std::array<ResourceWeights, 4> weights_;
	double comfortableCardsInHand = 5;
	double resourceBase = 1.2;
	double meadowVisibleDiscount = 0.5;
	double anyResourceCoef = 1.12;
	double potentialQuestDiscount = 0.3;
	double slotWinPointsValue = 2.0;
	double resourceRecountIters = 3;
	double resourceWeightDiscount = 1.4;

	std::string to_string() const {
		std::stringstream ss;
		ss << std::setprecision(3);
		for (const auto& w: weights_) {
			ss << w.to_string() << " | ";
		}
		ss << comfortableCardsInHand << " ";
		ss << resourceBase << " ";
		ss << meadowVisibleDiscount << " ";
		ss << anyResourceCoef << " ";
		ss << potentialQuestDiscount << " ";
		ss << slotWinPointsValue << " ";
		ss << resourceRecountIters << " ";
		ss << resourceWeightDiscount << " ";

		return ss.str();
	}

	std::string serialize() const {
		std::stringstream ss;
		ss << std::setprecision(4);
		for (const auto& w: weights_) {
			ss << w.wood << " ";
			ss << w.smola << " ";
			ss << w.stone << " ";
			ss << w.berry << " ";
			ss << w.card << " ";
			ss << w.winPoints << " ";
		}
		ss << comfortableCardsInHand << " ";
		ss << resourceBase << " ";
		ss << meadowVisibleDiscount << " ";
		ss << anyResourceCoef << " ";
		ss << potentialQuestDiscount << " ";
		ss << slotWinPointsValue << " ";
		ss << resourceRecountIters << " ";
		ss << resourceWeightDiscount << " ";

		return ss.str();
	}

	void parse(const std::string& s) {
		std::istringstream ss(s);
		for (auto& w: weights_) {
			ss >> w.wood;
			ss >> w.smola;
			ss >> w.stone;
			ss >> w.berry;
			ss >> w.card;
			ss >> w.winPoints;
		}
		ss >> comfortableCardsInHand;
		ss >> resourceBase;
		ss >> meadowVisibleDiscount;
		ss >> anyResourceCoef;
		ss >> potentialQuestDiscount;
		ss >> slotWinPointsValue;
		ss >> resourceRecountIters;
		ss >> resourceWeightDiscount;
	}

	void mutate() {
		static std::random_device r;
	    static std::default_random_engine rng(r());
		double magnitude = exp(((double)(rng() % 11) - 5) / 40.0);

		if (rng() % 2) {
			int idx = rng() % 4;
			int resourceIdx = rng() % 6;
			if (resourceIdx < 4) {
				for (auto& w: weights_) {
					*(&w.wood + resourceIdx) *= magnitude;
				}
			} else {
				double* ptr = &(weights_[idx].wood);
				ptr[resourceIdx] *= magnitude;
			}
		} else {
			double* ptr = &comfortableCardsInHand;
			ptr[rng() % 8] *= magnitude;
		}
	}

	void mutate(int times) {
		for (int i = 0; i < times; i++) {
			mutate();
		}
	}
};

class SimpleBot
{
public:
	SimpleBot(const BotParams& params, const GameEngine& ge);

	[[nodiscard]] Resource chooseResourcesToPay(int anyResources) const;
	[[nodiscard]] Resource chooseResourcesToGain(int anyResources) const;
	[[nodiscard]] Resource chooseResourcesToGain(const std::vector<Resource> resources) const;

	[[nodiscard]] std::vector<size_t> chooseCardsToDiscard(int amount) const;
	
	[[nodiscard]] size_t chooseBestCardToPlay(const std::vector<const Card*>& card, DiscountType discount) const;
	[[nodiscard]] std::vector<double> measureCardsInCityToSac(bool refundPrice = true, int dontSacCardId = -1) const;

	[[nodiscard]] double measureResourcesToGain(const Resource& resources) const;
	[[nodiscard]] double measureCardToPlay(const Card* card, DiscountType discount) const;
	[[nodiscard]] const std::vector<std::pair<double, size_t>>& measureCardsInHandToPlay() const;
	[[nodiscard]] const std::vector<std::pair<double, size_t>>& measureCardsInHandPotential() const;
	[[nodiscard]] const std::vector<std::pair<double, size_t>>& measureCardsInMeadowToPlay() const;
	[[nodiscard]] const std::vector<std::pair<double, size_t>>& measureCardsInMeadowPotential() const;

	void prepareWeights();
	void clearHandPrecalc();
	void evalHandPrecalc();

	std::string renderState() const;

	[[nodiscard]] std::vector<std::pair<double, size_t>> measureCardsPotential(const std::vector<const Card*>& cards) const;
	[[nodiscard]] std::vector<std::pair<double, size_t>> measureCardsToPlay(const std::vector<const Card*>& cards, DiscountType discount = DiscountType::NoneDiscount) const;
private:
	[[nodiscard]] double measureCardPotential(const Card* card) const;

	[[nodiscard]] Resource measureCardPlayEffect(const Card* card) const;
	[[nodiscard]] double measureCardLaterProcEffect(const Card* card) const;
	[[nodiscard]] double measureCardCitySlot(const Card* card) const;
	[[nodiscard]] double measureCardForQuests(const Card* card, int wannaPlay) const;
	[[nodiscard]] double measureCardForPurples(const Card* card, bool assumeVisibleCards) const;
	[[nodiscard]] double measureCardSynergies(const Card* card, int wannaPlay, bool assumeVisibleCards) const;
	[[nodiscard]] double measureCardResidents(const Card* card) const;
	[[nodiscard]] Resource imaginePotentialPrice(const Card* card, DiscountType discount, bool assumeVisibleCards) const;
	
	const GameEngine& ge;
	std::vector<std::pair<double, size_t>> cardsInHandPlayValues;
	std::vector<std::pair<double, size_t>> cardsInHandPotentials;
	std::vector<std::pair<double, size_t>> cardsInMeadowPlayValues;
	std::vector<std::pair<double, size_t>> cardsInMeadowPotentials;
	std::array<double, 100> visibleCards;
	std::array<std::vector<int>, 2> questCardsLeft;

	std::array<ResourceWeights, 4> weights_;
	int comfortableCardsInHand = 5;
	double resourceBase = 1.2;
	double meadowVisibleDiscount = 0.5;
	double anyResourceCoef = 1.12;
	double potentialQuestDiscount = 0.3;
	double slotWinPointsValue = 2.0;
	double resourceWeightDiscount = 1.4;
	size_t resourceRecountIters = 3;
	size_t occupiedCitySlotsTh = 10;

	Resource deployPrice = Resource{ .winPoints = 3 };

	ResourceWeights preparedWeights_;
	ResourceWeights demand_;
};
