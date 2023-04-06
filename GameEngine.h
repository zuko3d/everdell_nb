#pragma once

#include <random>
#include <vector>

#include "GameState.h"
#include "SimpleBot.h"

class GameEngine
{
public:
	// GameEngine(std::vector<SimpleBot*> bots_);

	void deployWorker(FieldAction& fieldAction);
	void deployWorker(CardInCity* cardInCity);
	
	void drawFromDeck(int player, size_t amount);
	void drawFromDeck(size_t amount);

	void chooseAndDrawFromMeadow(size_t amount);
	void chooseAndSacFromCity(size_t amount, bool onlyCreature = false, bool onlyBuilding = false);
	int chooseAndDiscard(size_t amount);
	int chooseAndAwardCardsToOpp(size_t amount);
	void sacFromCity(const Card* card);
	void sacFromCity(size_t idx);

	void registerDiscount(DiscountType discount);
	void registerTrigger(TriggerType trigger);

	// void awardWp(int player, int amount);
	void awardWp(int amount);
	void awardResources(int player, Resource resources);
	void awardResources(const Resource& resources);
	void spendResources(Resource resources);
	void awardCard(const Card* card);
	void awardCard(int player, const Card* card);
	void awardCardFromCity(int receiverPlayer, int idxInCity);

	void popDeck();

	void doTurn();
	void playGame(int seed);
	
	void procEffect(CardInCity* card);
	Resource imagineProcEffect(const CardInCity* card, int player) const;
	void procEffect(const Card* card);
	Resource imaginePlayEffect(const Card* card) const;
	void deployEffect(CardInCity* card);

	void playCard(const Card& card, DiscountType discount = DiscountType::NoneDiscount);
	void playCardFromMeadow(int idx, DiscountType discount = DiscountType::NoneDiscount);
	void playCardFromHand(int idx, DiscountType discount = DiscountType::NoneDiscount);
	
	void discard(size_t cardIndex);
	void discard(std::vector<size_t> cardIndex);

	void dropCardFromMeadow(size_t idx);
	void putToGraveyard(const Card* card);

	void resetGs(int seed);

	bool isCardInEffectStack(const Card*) const;

	const GameState& getGs() const;
	const PlayerState& getPlayer(int player) const;
	const PlayerState& getPlayer() const;
	const int getActivePlayer() const;
	const SimpleBot* getBot() const;
	const int nextPlayer() const;
	CardInCity* getCOI();
	const CardInCity* getCOI() const;
	
	void scorePoints();

	void setBots(std::vector<SimpleBot*> bots_);

	void renderLogToFile(const std::string& fname);
	void log(const std::string& message, const Card* card = nullptr);
private:
	void putToCity(const Card* card, int player);
	void removeFromCity(int idx);

	void log(const std::string& message, const Resource& res);
	void log(const std::string& message, const FieldAction& fa);
	void log(const std::string& message, const Quest& q);
	void logState();
	void logBf();

	std::vector<SimpleBot*> bots;
	GameState gs;
	mutable int activePlayer;

	std::vector<CardInCity*> cardOfInterest;

	std::array<std::vector<std::vector<std::string>>, 2> gameLog;
	std::array<std::vector<std::string>, 2> statesLog;
	std::vector<std::string> bfLog;

	std::array<std::array<std::array<int, 100>, 4>, 2> cardsInCityBySeasonEnd;
	std::array<std::array<std::array<int, 100>, 4>, 2> cardsPlayedBySeason;

	std::default_random_engine rng;
	int seed_ = -1;
};

