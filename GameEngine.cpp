#include "GameEngine.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <fstream>
#include <iostream>
#include <variant>
#include <random>
#include <string>
#include <stdexcept>
#include "Utils.h"

static std::vector<std::string> seasonName{ "Зима", "Весна", "Лето", "Осень" };

std::string genImageTag(const Card* card, const std::string& tooltip = "") {
	return "<img height=210px width=145px src =\"images/" + card->name + ".jpg\" title=\"" + tooltip + "\">";
}

std::string genImageTag(const FieldAction& fa) {
	return "<img height=105px width=145px src =\"images/" + std::to_string(fa.idx) + ".jpg\">";
}

std::string genImageTag(const Quest& q) {
	return "<img height=145px width=105px src =\"images/q" + std::to_string(q.idx) + ".jpg\">";
}

int canBeAttached(const Card& card, const std::vector<CardInCity>& city) {
	if (card.attachableToName.empty()) {
		return -1;
	}
	for (const auto& [c, idx] : enumerate(city)) {
		if (!c.hasAttachedCard && c.cardInfo->name == card.attachableToName) {
			return idx;
		}
	}
	return -1;
}

struct BrownAction {
	CardEffect& effect;
	int occupiedByPlayer = -1;
};

enum class ActionType {
	BaseAction,
	ForestAction,
	CityAction,
	Quest,
	ChangeSeason,
	PlayCardFromHand,
	PlayCardFromMeadow,
	Pass,
};

struct Action {
	ActionType type{ ActionType::Pass };
	size_t idx = -1;
};

// GameEngine::GameEngine(std::vector<SimpleBot*> bots_)
// 	: bots(std::move(bots_))
// {
// }

void GameEngine::deployWorker(FieldAction& fieldAction)
{
	PlayerState& playerState = gs.players[activePlayer];
	assert(playerState.freeWorkers > 0);

	playerState.freeWorkers--;

	assert(fieldAction.slots > fieldAction.deployedWorkers.size());
	fieldAction.deployedWorkers.push_back(activePlayer);

	assert(playerState.resources >= fieldAction.price);

	log("", fieldAction);

	spendResources(fieldAction.price);	
	awardResources(fieldAction.profit);
}

void GameEngine::deployWorker(CardInCity* cardInCity) {
	log("deploy to city ", cardInCity->cardInfo);
	cardInCity->occupiedByWorker = true;
	gs.players[activePlayer].freeWorkers--;
	deployEffect(cardInCity);
}

void GameEngine::drawFromDeck(size_t amount)
{
	drawFromDeck(activePlayer, amount);
}

void GameEngine::drawFromDeck(int player, size_t amount)
{
	auto& hand = gs.players[player].hand;
	auto handSize = gs.players[player].handSize;
	while ((hand.size() < handSize) && (amount > 0)) {
		awardCard(gs.deck.back());
		popDeck();
		amount--;
	}
}

void GameEngine::chooseAndDrawFromMeadow(size_t amount)
{
	auto& playerState = gs.players[activePlayer];

	for (size_t i = 0; i < amount; i++) {
		if (playerState.hand.size() < playerState.handSize) {
			const auto& measures = bots[activePlayer]->measureCardsInMeadowPotential();
			int idx = measures.back().second;
			playerState.hand.push_back(gs.meadow[idx]);
			gs.meadow[idx] = gs.deck.back();
			popDeck();
			playerState.resources.card++;
		}
	}

	bots[activePlayer]->clearHandPrecalc();
}

void GameEngine::chooseAndSacFromCity(size_t amount, bool onlyCreature, bool onlyBuilding)
{
	auto& playerState = gs.players[activePlayer];

	int discarded = 0;
	for (size_t sacced = 0; sacced < amount; sacced++) {
		const auto measures = bots[activePlayer]->measureCardsInCityToSac();
		auto order = argsort(measures);
		
		for (size_t i = 0; i < order.size(); i++) {
			size_t idx = order[i];
			const auto cardInCityInfo = playerState.city[idx].cardInfo;
			if (!onlyCreature || !cardInCityInfo->isBuilding) {
				if (!onlyBuilding || cardInCityInfo->isBuilding) {
					sacFromCity(idx);
					discarded++;
					break;
				}
			}
		}
	}
	assert(discarded == amount);
}

int GameEngine::chooseAndDiscard(size_t amount)
{
	const auto& ps = gs.players[activePlayer];
	if (amount > ps.hand.size()) {
		amount = ps.hand.size();
		while (ps.hand.size() > 0) {
			discard(0);
		}
	} else {
		discard(bots[activePlayer]->chooseCardsToDiscard(amount));
	}

	bots[activePlayer]->clearHandPrecalc();
	return amount;
}

int GameEngine::chooseAndAwardCardsToOpp(size_t amount)
{
	auto& ps = gs.players[activePlayer];
	auto& oppPs = gs.players[(activePlayer + 1) % gs.players.size()];
	amount = std::min(amount, oppPs.handSize - oppPs.hand.size());
	amount = std::min(amount, ps.hand.size());

	if (amount > 0) {
		auto indices = bots[activePlayer]->chooseCardsToDiscard(amount);
		std::sort(indices.begin(), indices.end());
		for (size_t i = 0; i < amount; i++) {
			const auto idx = indices[i] - i;
			oppPs.hand.push_back(ps.hand[idx]);
			oppPs.resources.card++;
			ps.hand.erase(ps.hand.begin() + idx);
			ps.resources.card--;
		}
		bots[activePlayer]->clearHandPrecalc();
	}
	
	return amount;
}

void GameEngine::sacFromCity(const Card* card)
{
	auto& city = gs.players[activePlayer].city;
	const auto iter = std::find_if(city.begin(), city.end(), [&](const CardInCity& c) { return c.cardInfo->idx == card->idx; });
	if (iter != city.end()) {
		sacFromCity(std::distance(city.begin(), iter));
	} else {
		throw std::logic_error("Nothing to sac!");
	}
}

void GameEngine::sacFromCity(size_t idx) {
	putToGraveyard(gs.players[activePlayer].city[idx].cardInfo);
	removeFromCity(idx);
}

void GameEngine::registerDiscount(DiscountType discount)
{
	gs.players[activePlayer].availableDiscounts[discount] = true;
}

void GameEngine::registerTrigger(TriggerType trigger)
{
	gs.players[activePlayer].availableTriggers[trigger] = true;
}

// void GameEngine::awardWp(int player, int amount)
// {
// 	gs.players[player].resources.winPoints += amount;
// }

void GameEngine::awardWp(int amount)
{
	gs.players[activePlayer].resources.winPoints += amount;
}

void GameEngine::awardResources(const Resource& resources)
{
	awardResources(activePlayer, resources);
}

void GameEngine::spendResources(Resource resources)
{
	auto& ps = gs.players[activePlayer];
	if (resources.card > 0) {
		chooseAndDiscard(resources.card);
		resources.card = 0;
	}
	if (resources.card < 0) {
		drawFromDeck(-resources.card);
		resources.card = 0;
	}
	if (resources.anyResource > 0) {
		ps.resources -= bots[activePlayer]->chooseResourcesToPay(resources.anyResource);
		resources.anyResource = 0;
	}
	if (resources.anyResource < 0) {
		ps.resources += bots[activePlayer]->chooseResourcesToGain(-resources.anyResource);
		
		resources.anyResource = 0;
	}
	ps.resources -= resources;
}

void GameEngine::awardCard(const Card* card)
{
	awardCard(activePlayer, card);
}

void GameEngine::awardCard(int player, const Card* card)
{
	auto& ps = gs.players[player];
	assert(ps.handSize > ps.hand.size());
	ps.hand.push_back(card);
	ps.resources.card++;
	bots[activePlayer]->clearHandPrecalc();
	log("draw", card);
}

void GameEngine::awardCardFromCity(int receiverPlayer, int idxInCity) {
	auto& ps = gs.players[activePlayer];
	const auto& cardInCity = ps.city[idxInCity];

	putToCity(cardInCity.cardInfo, receiverPlayer);
	removeFromCity(idxInCity);
}

void GameEngine::awardResources(int player, Resource resources)
{
	auto& ps = gs.players[player];
	if (resources.card > 0) {
		drawFromDeck(resources.card);
	}
	if (resources.card < 0) {
		chooseAndDiscard(-resources.card);
	}
	if (resources.anyResource > 0) {
		resources += bots[player]->chooseResourcesToGain(resources.anyResource);
		resources.anyResource = 0;
	}
	if (resources.anyResource < 0) {
		resources -= bots[player]->chooseResourcesToPay(-resources.anyResource);
		resources.anyResource = 0;
	}

	log("award " + resources.to_string());
	resources.card = 0;
	ps.resources += resources;
}

void GameEngine::popDeck()
{
	gs.deck.pop_back();
	if (gs.deck.empty()) {
		rshuffle(gs.graveyard, rng);
		gs.deck = std::move(gs.graveyard);
		gs.graveyard.clear();
	}
}

void GameEngine::doTurn()
{
	// iterate over available actions
	auto& ps = gs.players[activePlayer];
	auto& bot = bots[activePlayer];

	assert(cardOfInterest.empty());
	ps.validate();
	bot->prepareWeights();

	gameLog[activePlayer].push_back({});
	logState();

	if (ps.passed) {
		log("already passed");
		return;
	}
	
	Action bestAction;

	double bestPts = 0;
	// deploy
	if (ps.freeWorkers > 0) {
		for (const auto& [action, idx]: enumerate(gs.baseActions)) {
			if (ps.season < 3 && action.idx > 30) {
				continue;
			}
			if (action.deployedWorkers.size() < action.slots) {
				if (ps.resources >= action.price) {
					double pts = bot->measureResourcesToGain(action.profit - action.price);
					if (pts > bestPts) {
						bestPts = pts;
						bestAction = Action{ActionType::BaseAction, idx};
					}
				}
			}
		}

		for (const auto& [action, idx] : enumerate(gs.forestActions)) {
			if (action.deployedWorkers.size() < action.slots) {
				if (ps.resources >= action.price) {
					double pts = bot->measureResourcesToGain(action.profit - action.price);
					if (pts > bestPts) {
						bestPts = pts;
						bestAction = Action{ ActionType::ForestAction, idx };
					}
				}
			}
		}

		if (ps.season > 1) {
			for (const auto& [action, idx]: enumerate(gs.quests)) {
				if (ps.resources >= action.imaginaryPrice) {
					auto imaginaryEffect = action.imaginaryEffect(*this);
					const auto& oppPs = getPlayer(nextPlayer());
					if (oppPs.freeWorkers > 0 && action.cardsLeft(oppPs) <= 0) {
						imaginaryEffect = imaginaryEffect * 2;
					}
					double pts = bot->measureResourcesToGain(imaginaryEffect - action.imaginaryPrice);
					// log("q" + std::to_string(idx) + ": " + std::to_string(pts));
					if (pts > bestPts) {
						if (action.cardsLeft(ps) <= 0) {
							// log("+");
							bestPts = pts;
							bestAction = Action{ ActionType::Quest, idx };
						}
					}
				}
			}
		}

		for (const auto& [cardInCity, idx]: enumerate(ps.city)) {
			if (cardInCity.cardInfo->deployable && !cardInCity.occupiedByWorker) {
				double pts = bot->measureResourcesToGain(cardInCity.cardInfo->imagineDeployEffect(*this));
				if (pts > bestPts) {
					bestPts = pts;
					bestAction = Action{ ActionType::CityAction, idx };
				}
			}
		}
	}

	// play card
	for (const auto& [cardInHand, idx]: enumerate(ps.hand)) {
		if (ps.canPlayCard(cardInHand)) {
			double pts = bot->measureCardToPlay(cardInHand, NoneDiscount) - bot->measureResourcesToGain({.card = 1});
			if (pts > bestPts) {
				bestPts = pts;
				bestAction = Action{ ActionType::PlayCardFromHand, idx };
			}
		}
	}

	for (const auto& [cardInMeadow, idx]: enumerate(gs.meadow)) {
		if (ps.canPlayCard(cardInMeadow)) {
			double pts = bot->measureCardToPlay(cardInMeadow, NoneDiscount);
			if (pts > bestPts) {
				bestPts = pts;
				bestAction = Action{ ActionType::PlayCardFromMeadow, idx };
			}
		}
	}

	// ChangeSeason
	if ((bestAction.type == ActionType::Pass) && ps.season < 3) {
		bestAction.type = ActionType::ChangeSeason;
	}

	ps.validate();

	// pass?
	switch (bestAction.type) {
	case ActionType::BaseAction:
		deployWorker(gs.baseActions[bestAction.idx]);
		break;
	case ActionType::ForestAction:
		deployWorker(gs.forestActions[bestAction.idx]);
		break;
	case ActionType::CityAction: {
		deployWorker(&ps.city[bestAction.idx]);
		break; 
	}
	case ActionType::Quest: {
		log("", gs.quests[bestAction.idx]);
		auto& quest = gs.quests[bestAction.idx];
		quest.effect(*this);
		ps.finishedQuests.push_back(gs.quests[bestAction.idx]);
		ps.freeWorkers--;
		gs.quests.erase(gs.quests.begin() + bestAction.idx);
		break;
	}
	case ActionType::PlayCardFromHand: {
		playCardFromHand(bestAction.idx);
		break;
	}
	case ActionType::PlayCardFromMeadow: {
		playCardFromMeadow(bestAction.idx);
		break;
	}
	case ActionType::ChangeSeason: {
		log(seasonName[ps.season] + " -> " + seasonName[ps.season + 1]);
		if (ps.availableTriggers[TriggerType::Tower]) {
			Card::allTriggers[TriggerType::Tower](*this);
		}

		ps.season++;
		for (auto& action : gs.baseActions) {
			auto& v = action.deployedWorkers;
			v.erase(std::remove(v.begin(), v.end(), activePlayer), v.end());
		}

		for (auto& action : gs.forestActions) {
			auto& v = action.deployedWorkers;
			v.erase(std::remove(v.begin(), v.end(), activePlayer), v.end());
		}

		for (auto& action : ps.city) {
			action.occupiedByWorker = false;
		}

		ps.freeWorkers = ps.season + 2;
		if (ps.season == 3) ps.freeWorkers++;

		if (ps.season == 1 || ps.season == 3) {
			std::vector<std::pair<int, CardInCity*>> procs;
			for (auto& cardInCity : ps.city) {
				if (cardInCity.cardInfo->color == Color::Green) {
					procs.push_back({cardInCity.cardInfo->greenProcOrder, &cardInCity});
				}
			}
			std::sort(procs.begin(), procs.end());
			for (const auto& [prio, card]: procs) {
				log("proc ", card->cardInfo);
				procEffect(card);
			}
		}
		if (ps.season == 2) {
			chooseAndDrawFromMeadow(2);
		}

		break;
	}
	case ActionType::Pass:
		log("pass");
		scorePoints();
		ps.passed = true;
		break;

	default:
		assert(false);
	}

	ps.validate();
	getPlayer(nextPlayer()).validate();
}

void GameEngine::playGame(int seed)
{
	resetGs(seed);

	bfLog.clear();

	activePlayer = 0;
	int round = 1;
	bool allPassed = false;
	while (!allPassed) {
		allPassed = true;
		logBf();
		for (activePlayer = 0; activePlayer < gs.players.size(); activePlayer++) {
			if (!getPlayer().passed) {
				allPassed = false;
			}
			doTurn();
		}
		round++;
	}
}

void GameEngine::procEffect(CardInCity* card)
{
	cardOfInterest.push_back(card);
	card->cardInfo->effect(*this);
	cardOfInterest.pop_back();
}

Resource GameEngine::imagineProcEffect(const CardInCity* card, int player) const {
	int curActive = activePlayer;
	activePlayer = player;
	const auto ret = card->cardInfo->imagineProcEffect(*this);
	activePlayer = curActive;
	return ret;
}

void GameEngine::procEffect(const Card* card)
{
	cardOfInterest.push_back(nullptr);
	card->effect(*this);
	cardOfInterest.pop_back();
}

Resource GameEngine::imaginePlayEffect(const Card* card) const {
	Resource effect;
	if (card->color != Red) {
		effect = card->imagineProcEffect(*this);
	}
	const auto& ps = getPlayer();
	if (ps.availableTriggers[Historic])	{
		effect.card++;
	}
	if (card->isBuilding) {
		if (ps.availableTriggers[Court]) {
			effect.anyResource++;
		}
	} else if (ps.availableTriggers[Lavochnik]) {
		effect.berry++;
	}

	return effect;
}

void GameEngine::deployEffect(CardInCity* card)
{
	cardOfInterest.push_back(card);
	card->cardInfo->deployEffect(*this);
	cardOfInterest.pop_back();
}

void GameEngine::playCard(const Card& card, DiscountType discount)
{
	auto& ps = gs.players[activePlayer];

	auto canPlay = ps.canPlayCard(&card, discount);
	assert(canPlay != CantPlay);

	const auto freeIdx = ps.canPlayCreatureForShield(&card);
	bool shieldUsed = false;
	if (freeIdx > -1 && ((canPlay & CanPlay::OnlyForShield) || !(canPlay & CanPlay::OnlyWithSac))) {
		log("use shield");
		ps.city[freeIdx].shieldUsed = true;
		shieldUsed = true;
	} 
	if (!shieldUsed || (canPlay & CanPlay::OnlyWithSac)) {
		if (discount == DiscountType::NoneDiscount) {
			discount = ps.chooseBestDiscount(card);
		}
		auto discountedPrice = ps.applyDiscount(discount, card);
		if (!(ps.resources >= discountedPrice)) {
			assert(freeIdx >= 0);
			ps.city[freeIdx].shieldUsed = true;
			discountedPrice = Resource{};
		}
		ps.resources -= discountedPrice;
		if (discount == DiscountType::Traktirshik) {
			sacFromCity(Card::traktirshikCard);
			ps.availableDiscounts[discount] = false;
		} else if (discount == DiscountType::Crane) {
			sacFromCity(Card::craneCard);
			ps.availableDiscounts[discount] = false;
		} else if (discount == DiscountType::Inventor) {
			sacFromCity(Card::inventorCard);
			ps.availableDiscounts[discount] = false;
		} else if (discount == DiscountType::Dungeon) {
			chooseAndSacFromCity(1, true);
			ps.availableDiscounts[discount] = false;
		}
	}

	const auto availableTriggersAtStart = ps.availableTriggers;

	if (card.idx == Card::razvaliniCard->idx) {
		procEffect(Card::razvaliniCard);
	}

	log("put to city ", &card);
	putToCity(&card, activePlayer);

	const auto cardInCity = &ps.city.back();
	if (card.color == Color::Green || card.color == Color::Grey && card.idx != Card::razvaliniCard->idx) {
		procEffect(cardInCity);
	}

	if (availableTriggersAtStart[TriggerType::Historic]) {
		Card::allTriggers[TriggerType::Historic](*this);
	}
	if (card.isBuilding) {
		if (availableTriggersAtStart[TriggerType::Court]) {
			Card::allTriggers[TriggerType::Court](*this);
		}
	} else {
		if (availableTriggersAtStart[TriggerType::Lavochnik]) {
			Card::allTriggers[TriggerType::Lavochnik](*this);
		}
	}

	// Blue cards register triggers, so register them after triggering current triggers
	if (card.color == Color::Blue) {
		procEffect(cardInCity);
	}

	ps.validate();
}

void GameEngine::playCardFromMeadow(int idx, DiscountType discount)
{
	const auto card = gs.meadow.at(idx);
	log(card->name +  " from meadow");
	gs.meadow[idx] = gs.deck.back();
	popDeck();
	playCard(*card, discount);
}

void GameEngine::playCardFromHand(int idx, DiscountType discount)
{
	auto& hand = gs.players[activePlayer].hand;
	const auto card = hand.at(idx);
	log(card->name + " from hand");
	hand.erase(hand.begin() + idx);
	gs.players[activePlayer].resources.card--;
	bots[activePlayer]->clearHandPrecalc();
	playCard(*card, discount);
}

void GameEngine::putToGraveyard(const Card* card)
{
	log("-> GY ", card);
	gs.graveyard.push_back(card);
}

void GameEngine::discard(size_t cardIndex)
{
	auto& hand = gs.players[activePlayer].hand;
	putToGraveyard(hand[cardIndex]);
	hand.erase(hand.begin() + cardIndex);
	gs.players[activePlayer].resources.card--;
	bots[activePlayer]->clearHandPrecalc();
}

void GameEngine::discard(std::vector<size_t> cardIndex)
{
	std::sort(cardIndex.begin(), cardIndex.end());
	for (int i = 0; i < cardIndex.size(); i++) {
		discard(cardIndex[i] - i);
	}
}

void GameEngine::dropCardFromMeadow(size_t idx)
{
	putToGraveyard(gs.meadow[idx]);
	gs.meadow[idx] = gs.deck.back();
	popDeck();
}

void GameEngine::resetGs(int seed)
{
	seed_ = seed;
	rng = std::default_random_engine(seed);
	
	gs.deck.clear();
	for (const auto& c : Card::allCardsInGame) {
		for (int i = 0; i < c.amountInGame; i++) {
			gs.deck.emplace_back(&c);
		}
	}
	rshuffle(gs.deck, rng);

	gs.baseActions = genBaseActions();
	gs.forestActions = genForestActions(rng);

	gs.quests = genAllQuests(rng);
	
	gs.graveyard.clear();

	gs.meadow.clear();
	gs.meadow.reserve(8);
	for (int i = 0; i < 8; i++) {
		gs.meadow.push_back(gs.deck.back());
		popDeck();
	}

	gs.players = std::vector<PlayerState>(bots.size());
	
	for (activePlayer = 0; activePlayer < gs.players.size(); activePlayer++) {
		gs.players[activePlayer].city.reserve(30);
		gameLog[activePlayer].push_back({});
		drawFromDeck(activePlayer, 5 + activePlayer);
		gs.players[activePlayer].validate();
		gameLog[activePlayer].pop_back();
	}
}

bool GameEngine::isCardInEffectStack(const Card* card) const
{
	return std::find_if(cardOfInterest.begin(), cardOfInterest.end(), [=](const CardInCity* c)
		{
			return c != nullptr && c->cardInfo->idx == card->idx;
		}) != cardOfInterest.end();
}

const GameState& GameEngine::getGs() const
{
	return gs;
}

const PlayerState& GameEngine::getPlayer(int player) const
{
	return gs.players[player];
}

const PlayerState& GameEngine::getPlayer() const
{
	return getPlayer(activePlayer);
}

const int GameEngine::getActivePlayer() const
{
	return activePlayer;
}

const SimpleBot* GameEngine::getBot() const
{
	return bots[activePlayer];
}

const int GameEngine::nextPlayer() const
{
	return (activePlayer + 1) % gs.players.size();
}

CardInCity* GameEngine::getCOI()
{
	return cardOfInterest.back();
}

const CardInCity* GameEngine::getCOI() const
{
	return cardOfInterest.back();
}

void GameEngine::scorePoints()
{
	const auto& ps = getPlayer();
	for (const auto& c: ps.city) {
		awardWp(c.cardInfo->winPoints);
		awardWp(c.resources.winPoints);
		if (c.cardInfo->color == Color::Purple) {
			procEffect(const_cast<CardInCity*>(&c));
		}
	}
}

void GameEngine::setBots(std::vector<SimpleBot*> bots_)
{
	bots = std::move(bots_);
}

void GameEngine::renderLogToFile(const std::string& fname)
{
#ifndef DEBUG
	std::cerr << "Loggin disabled due to no DEBUG mode!" << std::endl;
	return;
#endif
	auto fout = std::ofstream(fname);

	fout << "<html><body>seed: " << seed_ << "<br><table border=1>";

	for (size_t turn = 0; turn < gameLog[0].size(); turn++)	{
		fout << "<tr><td colspan=\"5\">";
		
		fout << "<details ";
		if ((turn == 0) || (bfLog[turn] != bfLog[turn - 1])){
			fout << "open";
		}
		fout << "><summary>BF</summary>" << bfLog[turn] << "</details></td></tr>";
		fout << "<tr><td>" << turn + 1 << "</td>";
		for (size_t p = 0; p < gs.players.size(); p++) {
			fout << "<td style=\"min-width: 730px;\"><details ";
			if (turn == 0) {
				fout << "open";
			}
			fout << "><summary>Player State</summary>" << statesLog[p][turn] << "</details></td><td style=\"max-width: 160px;\">";
			
			for (const auto& msg : gameLog[p][turn]) {
				fout << msg << "<br>";
			}
			fout << "</td>";
			
		}

		fout << "</tr>";
	}

	fout << "</table></body></html>";
}

void GameEngine::log(const std::string& message, const Card* card)
{
#ifndef DEBUG
	return;
#endif
	if (card == nullptr) {
		gameLog[activePlayer].back().push_back(message);
	} else {
		gameLog[activePlayer].back().push_back(message + "<br><img height=210px width=145px src =\"images/" + card->name + ".jpg\">");
	}
}

void GameEngine::putToCity(const Card* card, int player)
{
	auto& playerState = gs.players[player];

	int attachable = canBeAttached(*card, playerState.city);
	if (attachable > -1) {
		playerState.city[attachable].hasAttachedCard = true;
	} else {
		playerState.freeCitySlots -= card->occupiedCitySlots;
	}

	playerState.cityLookup[card->idx]++;
	playerState.coloredCardsLookup[card->color]++;

	playerState.city.emplace_back(card);
	if (attachable > -1) {
		playerState.city.back().hasAttachedCard = true;
	}
}

void GameEngine::removeFromCity(int idx) {
	auto& ps = gs.players[activePlayer];
	const auto& cardInCity = ps.city[idx];
	bool wasAttached = false;
	if (cardInCity.hasAttachedCard) {
		wasAttached = true;
		const auto partnerIdx = cardInCity.cardInfo->attachableToCard->idx;
		for (auto& c: ps.city) {
			if (c.hasAttachedCard && c.cardInfo->idx == partnerIdx) {
				c.hasAttachedCard = false;
				break;
			}
		}
		// throw std::logic_error("wanna sac attached card");
	}
	ps.coloredCardsLookup[cardInCity.cardInfo->color]--;
	ps.cityLookup[cardInCity.cardInfo->idx]--;

	if (cardInCity.cardInfo->idx == Card::judgeCard->idx) {
		ps.availableDiscounts[DiscountType::Judge] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::traktirshikCard->idx) {
		ps.availableDiscounts[DiscountType::Traktirshik] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::craneCard->idx) {
		ps.availableDiscounts[DiscountType::Crane] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::dungeonCard->idx) {
		ps.availableDiscounts[DiscountType::Dungeon] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::courtCard->idx) {
		ps.availableTriggers[TriggerType::Court] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::historicCard->idx) {
		ps.availableTriggers[TriggerType::Historic] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::lavochnikCard->idx) {
		ps.availableTriggers[TriggerType::Lavochnik] = false;
	}
	else if (cardInCity.cardInfo->idx == Card::towerCard->idx) {
		ps.availableTriggers[TriggerType::Tower] = false;
	}

	if (!wasAttached) {
		ps.freeCitySlots += cardInCity.cardInfo->occupiedCitySlots;
	}
	ps.city.erase(ps.city.begin() + idx);
}

void GameEngine::log(const std::string& message, const Resource& res)
{
#ifndef DEBUG
	return;
#endif
	gameLog[activePlayer].back().push_back(message + res.to_string());
}

void GameEngine::log(const std::string& message, const FieldAction& fa)
{
#ifndef DEBUG
	return;
#endif
	gameLog[activePlayer].back().push_back(message + "<br>" + genImageTag(fa));
}

void GameEngine::log(const std::string& message, const Quest& q)
{
#ifndef DEBUG
	return;
#endif
	gameLog[activePlayer].back().push_back(message + "<br>" + genImageTag(q));
}

void GameEngine::logState()
{
#ifndef DEBUG
	return;
#endif
	const auto& ps = getPlayer();
	std::string ret;
	ret.reserve(1000);

	ret += ps.resources.to_string() + "<br>";
	ret += "season: " + seasonName[ps.season] + "<br>";
	ret += getBot()->renderState();
	ret += "free workers: " + std::to_string(ps.freeWorkers) + "<br>";
	ret += "hand:<br>";
	for (int i = 0; i < ps.hand.size(); i++) {
		if (i > 0 && i % 5 == 0) {
			ret += "<br>";
		}
		ret += genImageTag(ps.hand[i]);
	}
	ret += "<br>";

	ret += "city:<br>";
	for (int i = 0; i < ps.city.size(); i++) {
		if (i > 0 && i % 5 == 0) {
			ret += "<br>";
		}
		const auto& cardInCity = ps.city[i];
		std::string tooltip = "";
		if (cardInCity.hasAttachedCard) tooltip += "attached ";
		if (cardInCity.occupiedByWorker) tooltip += "deployed ";
		if (cardInCity.shieldUsed) tooltip += "shieldUsed ";
		tooltip += cardInCity.resources.to_string();
		ret += genImageTag(cardInCity.cardInfo, tooltip);
	}

	ret += "<br>quests:<br>";
	for (int i = 0; i < ps.finishedQuests.size(); i++) {
		ret += genImageTag(ps.finishedQuests[i]);
	}

	statesLog[activePlayer].emplace_back(std::move(ret));
}

void GameEngine::logBf()
{
#ifndef DEBUG
	return;
#endif

	std::string ret;
	ret.reserve(1000);

	ret += "<table border=1><tr><td>";
	for (int i = 0; i < gs.meadow.size(); i++) {
		ret += genImageTag(gs.meadow[i]);
	}
	ret += "</td><td>";
	int pos = 0;
	for (const auto& fa: gs.baseActions) {
		if (fa.deployedWorkers.size() < fa.slots && fa.slots < 10) {
			if ((pos > 0) && (pos % 4 == 0)) {
				ret += "<br>";
			}
			ret += genImageTag(fa);
			pos++;
		}
	}
	for (const auto& fa : gs.forestActions) {
		if (fa.deployedWorkers.size() < fa.slots && fa.slots < 10) {
			if ((pos > 0) && (pos % 4 == 0)) {
				ret += "<br>";
			}
			ret += genImageTag(fa);
			pos++;
		}
	}
	ret += "</td></tr></table><br>";

	for (int i = 0; i < gs.quests.size(); i++) {
		ret += genImageTag(gs.quests[i]);
	}

	bfLog.emplace_back(std::move(ret));
}
