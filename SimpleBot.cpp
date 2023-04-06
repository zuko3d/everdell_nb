#include "SimpleBot.h"

#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

#include "GameEngine.h"
#include "Utils.h"

SimpleBot::SimpleBot(const BotParams& params, const GameEngine& ge_)
	: weights_(params.weights_)
	, comfortableCardsInHand(params.comfortableCardsInHand)
	, resourceBase(params.resourceBase)
	, meadowVisibleDiscount(params.meadowVisibleDiscount)
	, anyResourceCoef(params.anyResourceCoef)
	, potentialQuestDiscount(params.potentialQuestDiscount)
	, slotWinPointsValue(params.slotWinPointsValue)
	, resourceRecountIters(params.resourceRecountIters)
	, resourceWeightDiscount(params.resourceWeightDiscount)
	, ge(ge_)
{
}

Resource SimpleBot::chooseResourcesToPay(int anyResources) const
{
	const PlayerState& ps = ge.getPlayer();
	Resource ret;
	ResourceWeights weights = preparedWeights_;
	while (anyResources > 0) {
		auto weightsIter = &weights.wood;
		auto psResourceIter = &ps.resources.wood;
		auto retIter = &ret.wood;
		int bestI = -1;
		int bestWeight = 10000;
		for (int i = 0; i < 4; i++) {
			if (*weightsIter < bestWeight && *psResourceIter > *retIter) {
				bestI = i;
				bestWeight = *weightsIter;
			}

			weightsIter++;
			psResourceIter++;
			retIter++;
		}
		if (bestI == -1) {
			throw std::logic_error("can't pay!");
		} else {
			(&weights.wood)[bestI] = ((double)(&weights.wood)[bestI]) * resourceBase;
			(& ret.wood)[bestI]++;
		}
		anyResources--;
	}

	return ret;
}

Resource SimpleBot::chooseResourcesToGain(int anyResources) const
{
	const PlayerState& ps = ge.getPlayer();
	Resource ret;
	ResourceWeights weights = preparedWeights_;
	while (anyResources > 0) {
		auto weightsIter = &weights.wood;
		auto psResourceIter = &ps.resources.wood;
		auto retIter = &ret.wood;
		int bestI = 0;
		int bestWeight = *weightsIter;
		for (int i = 0; i < 4; i++) {
			if (*weightsIter > bestWeight) {
				bestI = i;
				bestWeight = *weightsIter;
			}

			weightsIter++;
			psResourceIter++;
			retIter++;
		}
		
		(& weights.wood)[bestI] = ((double)(&weights.wood)[bestI]) / resourceBase;
		(&ret.wood)[bestI]++;
		
		anyResources--;
	}

	return ret;
}

Resource SimpleBot::chooseResourcesToGain(const std::vector<Resource> resources) const
{
	Resource ret;
	double bestPts = 0;
	for (const auto& res: resources) {
		auto pts = measureResourcesToGain(res);
		if (pts > bestPts) {
			bestPts = pts;
			ret = res;
		}
	}
	return ret;
}

std::vector<size_t> SimpleBot::chooseCardsToDiscard(int amount) const
{
	std::vector<size_t> worstCards;
	for (int i = 0; i < amount; i++) {
		worstCards.push_back(cardsInHandPotentials[i].second);
	}
	return worstCards;
}

std::vector<std::pair<double, size_t>> SimpleBot::measureCardsToPlay(const std::vector<const Card*>& cards, DiscountType discount) const
{
	std::vector<std::pair<double, size_t>> ret;
	ret.reserve(cards.size());
	for (const auto& [c, idx]: enumerate(cards)) {
		ret.push_back({ measureCardToPlay(c, discount), idx });
	}
	std::sort(ret.begin(), ret.end());
	return ret;
}

double SimpleBot::measureCardForQuests(const Card* card, int wannaPlay) const
{
	const auto& ps = ge.getPlayer();
	double ret = 0.0;
	for (const auto& [q, idx]: enumerate(ge.getGs().quests)) {
		if (q.isCardForQuest(ps, card)) {
			const auto cardsLeft = questCardsLeft[0][idx] - 1;
			if (cardsLeft >= 0) {
				ret += (double) q.imaginaryEffect(ge).winPoints * pow(potentialQuestDiscount, cardsLeft);
			}
		}
	}

	return ret * preparedWeights_.winPoints;
}

double SimpleBot::measureCardForPurples(const Card* card, bool assumeVisibleCards) const {
	double ret = 0.0;
	const auto& ps = ge.getPlayer();
	for (const auto& c : ps.city) {
		if (c.cardInfo->color == Color::Purple) {
			ret += c.cardInfo->purpleValue(card) * preparedWeights_.winPoints;
		}
	}

	if (assumeVisibleCards) {
		for (const auto& c : ge.getGs().meadow) {
			if (c->color == Color::Purple) {
				if (!ps.hasSlotForCard(c)) {
					continue;
				}
				ret += c->purpleValue(card) * preparedWeights_.winPoints * meadowVisibleDiscount;
			}
		}

		for (const auto& c : ps.hand) {
			if (c->color == Color::Purple) {
				if (!ps.hasSlotForCard(c)) {
					continue;
				}
				ret += c->purpleValue(card) * preparedWeights_.winPoints * 0.5;
			}
		}
	}

	return ret;
}

double SimpleBot::measureCardSynergies(const Card* card, int wannaPlay, bool assumeVisibleCards) const {
	const auto& ps = ge.getPlayer();
	double ret = 0.0;

	if (card->synergyBase != nullptr) {
		const auto cardInCity = ps.getCardInCity(card->synergyBase);
		double coef = wannaPlay;
		if (card->color == Color::Green) {
			coef += ps.greenProcsLeft();
		}
		if (cardInCity != nullptr) {
			ret += measureResourcesToGain(card->imagineSynergyBonus(ge)) * coef;
		} else if (assumeVisibleCards) {
			ret += coef * (preparedWeights_ * card->imagineSynergyBonus(ge)) * std::min(1.0, visibleCards[card->synergyBase->idx]);
		}
	}

	for (const auto& d: card->synergyDuo) {
		const auto cardsInCity = ps.countCardInCity(d);
		if (cardsInCity > 0) {
			if (card->color == Color::Green) {
				const auto coef = ps.greenProcsLeft();

				const auto bonus = measureResourcesToGain(d->imagineSynergyBonus(ge));
				ret += bonus * coef * cardsInCity;

				if (assumeVisibleCards) {
					ret += coef * bonus * std::min(1.0, visibleCards[d->idx]);
				}
			}
		}
	}

	return ret;
}

double SimpleBot::measureCardResidents(const Card* card) const {
	double ret = 0.0;
	if (card->residentCard != nullptr && (!card->residentCard->isUnique || !ge.getPlayer().hasCardInCity(card->residentCard))) {
		// actually should account here only for usable creatures
		ret += (double)(card->residentCard->cost.berry * preparedWeights_.berry) * std::min(1.0, visibleCards[card->residentCard->idx]);
	}

	return ret;
}

Resource SimpleBot::imaginePotentialPrice(const Card* card, DiscountType discount, bool assumeVisibleCards) const {
	const auto& ps = ge.getPlayer();

	Resource actualPrice;
	if (discount == NoneDiscount) {
		actualPrice = ps.minimalPrice(card);
		if (assumeVisibleCards && actualPrice.cmc() > 0) {
			if (card->mainBuildingCard != nullptr) {
				const auto cardInCity = ps.getCardInCity(card->mainBuildingCard);
				if (cardInCity != nullptr) {
					if (!card->mainBuildingCard->isUnique && (visibleCards[card->mainBuildingCard->idx] > 0)) {
						return Resource{};
					}
				}
				else if (visibleCards[card->mainBuildingCard->idx] > 0) {
					return Resource{};
				}
			}
		}
	} else {
		actualPrice = ps.applyDiscount(discount, *card);
	}
	
	return actualPrice;
}

std::vector<std::pair<double, size_t>> SimpleBot::measureCardsPotential(const std::vector<const Card*>& cards) const
{
	std::vector<std::pair<double, size_t>> ret;
	ret.reserve(18);
	for (const auto& [c, idx]: enumerate(cards)) {
		ret.push_back({ measureCardPotential(c), idx });
	}
	std::sort(ret.begin(), ret.end());
	return ret;
}

size_t SimpleBot::chooseBestCardToPlay(const std::vector<const Card*>& cards, DiscountType discount) const
{
	return measureCardsToPlay(cards, discount).back().second;
}

std::vector<double> SimpleBot::measureCardsInCityToSac(bool refundPrice, int dontSacCardId) const
{
	const PlayerState& ps = ge.getPlayer();

	std::vector<double> ret;
	ret.reserve(18);
	for(const auto& c: ps.city) {
		if (c.cardInfo->idx == dontSacCardId || c.cardInfo->occupiedCitySlots == 0) {
			ret.push_back(1e9);
			continue;
		}

		double additionalValue = 0.0;

		if (c.hasAttachedCard) {
			// just never sac whis guy
			if (visibleCards[c.cardInfo->idx] > 0) {
				additionalValue += 3 * preparedWeights_.winPoints;
			} else {
				additionalValue += 1000 * preparedWeights_.winPoints;
			}
		}
		
		const Card* card = c.cardInfo;
		additionalValue += measureCardLaterProcEffect(card);
		if (!c.shieldUsed) {
			additionalValue += measureCardResidents(card);
		}
		additionalValue += measureCardForQuests(card, 0);
		additionalValue += measureCardForPurples(card, true);
		additionalValue += measureCardSynergies(card, 0, true);
		additionalValue -= measureCardCitySlot(card);
		
		ret.push_back(additionalValue - (refundPrice * measureResourcesToGain(c.cardInfo->cost)));
	}
	return ret;
}

double SimpleBot::measureResourcesToGain(const Resource& resources) const
{
	double ret = 0.0;
	if (resources.anyResource == 0) {
		ret += preparedWeights_ * resources;
	} else {
		auto tres = resources;
		tres.anyResource = 0;
		if (resources.anyResource < 0) {
			tres -= chooseResourcesToPay(-resources.anyResource);
		} else {
			tres += chooseResourcesToGain(resources.anyResource);
		}
		ret += preparedWeights_ * tres;
	}
	if (resources.card < 0)
	{
		const auto& ps = ge.getPlayer();
		int availableBuildSlots = ps.freeCitySlots;
		if (ps.availableDiscounts[DiscountType::Crane]) availableBuildSlots++;
		if (ps.availableDiscounts[DiscountType::Dungeon]) availableBuildSlots++;
		if (ps.availableDiscounts[DiscountType::Inventor]) availableBuildSlots++;
		if (ps.availableDiscounts[DiscountType::Traktirshik]) availableBuildSlots++;

		for (int i = 0; i < (-resources.card) && (i < cardsInHandPotentials.size()) && (i < availableBuildSlots); i++) {
			ret -= std::max(0.0, cardsInHandPotentials[i].first);
		}
	}
	if (resources.card > 0)	{
		int extraCards = std::max((int) 0, (ge.getPlayer().resources.card + resources.card - (int) ge.getPlayer().handSize));
		ret -= extraCards * preparedWeights_.card;
	}
	return ret;
}

double SimpleBot::measureCardToPlay(const Card* card, DiscountType discount) const
{
	if (!ge.getPlayer().hasSlotForCard(card)) {
		return -1e5;
	}
	
	Resource ret = measureCardPlayEffect(card) - imaginePotentialPrice(card, discount, false);

	double additionalValue = 0.0;
	additionalValue += measureCardLaterProcEffect(card);
	additionalValue += measureCardResidents(card);
	additionalValue += measureCardForQuests(card, 1);
	additionalValue += measureCardForPurples(card, false);
	additionalValue += measureCardSynergies(card, 1, false);
	additionalValue -= measureCardCitySlot(card);

	return measureResourcesToGain(ret) + additionalValue;
}

double SimpleBot::measureCardPotential(const Card* card) const
{
	if (!ge.getPlayer().hasSlotForCard(card)) {
		return -1e5;
	}
	
	Resource ret = measureCardPlayEffect(card) - imaginePotentialPrice(card, NoneDiscount, true);
	
	double additionalValue = 0.0;
	additionalValue += measureCardLaterProcEffect(card);
	additionalValue += measureCardResidents(card);
	additionalValue += measureCardForQuests(card, 1);
	additionalValue += measureCardForPurples(card, true);
	additionalValue += measureCardSynergies(card, 1, true);
	additionalValue -= measureCardCitySlot(card);
	
	return measureResourcesToGain(ret) + additionalValue;
	// return weights_[3] * ret + additionalValue;
}

double SimpleBot::measureCardCitySlot(const Card* card) const
{
	int potentialSlotsOccupied = std::min(card->occupiedCitySlots, (int)(card->attachableToCard == nullptr));
	return slotWinPointsValue * potentialSlotsOccupied * preparedWeights_.winPoints;
}

Resource SimpleBot::measureCardPlayEffect(const Card* card) const {
	const auto& ps = ge.getPlayer();

	Resource effect = ge.imaginePlayEffect(card);
	
	return effect;
}

double SimpleBot::measureCardLaterProcEffect(const Card* card) const {
	const auto& ps = ge.getPlayer();
	
	double delayedValue = 0.0;

	delayedValue += card->winPoints * preparedWeights_.winPoints;

	if (card->color == Color::Green) {
		Resource effect = card->imagineProcEffect(ge);
		if (ps.season < 3) {
			delayedValue += weights_[3] * effect;
		}
		if (ps.season < 1) {
			delayedValue += weights_[1] * effect;
		}
	}

	if (card->color == Color::Red) {
		if (ge.getPlayer().freeWorkers > 0) {
			delayedValue += preparedWeights_ * (card->imagineDeployEffect(ge) - deployPrice);
		}
		for (int season = ps.season + 1; season < 4; season++) {
			delayedValue += weights_[season] * (card->imagineLaterEffect(ge, season) - deployPrice);
		}
	}

	if (card->color == Color::Blue) {
		for (int season = ps.season + 1; season < 4; season++) {
			delayedValue += weights_[season] * card->imagineLaterEffect(ge, season);
		}
	}

	return delayedValue;
}

const std::vector<std::pair<double, size_t>>& SimpleBot::measureCardsInHandToPlay() const	{
	return cardsInHandPlayValues;
}

const std::vector<std::pair<double, size_t>>& SimpleBot::measureCardsInHandPotential() const
{
	return cardsInHandPotentials;
}

const std::vector<std::pair<double, size_t>>& SimpleBot::measureCardsInMeadowToPlay() const
{
	return cardsInMeadowPlayValues;
}

const std::vector<std::pair<double, size_t>>& SimpleBot::measureCardsInMeadowPotential() const
{
	return cardsInMeadowPotentials;
}

void SimpleBot::prepareWeights()
{
	const PlayerState& ps = ge.getPlayer();
	for (auto& c : visibleCards) {
		c = 0.0;
	}
	for (const auto& c : ps.hand) {
		visibleCards[c->idx]++;
	}
	for (const auto& c : ge.getGs().meadow) {
		visibleCards[c->idx] += meadowVisibleDiscount;
	}

	const auto& oppPs = ge.getPlayer(ge.nextPlayer());
	questCardsLeft[0].clear();
	questCardsLeft[1].clear();
	for (const auto& q: ge.getGs().quests){ 
		questCardsLeft[0].push_back(q.cardsLeft(ps));
		questCardsLeft[1].push_back(q.cardsLeft(oppPs));
	}

	preparedWeights_ = weights_[ps.season];
	double sum = weights_[ps.season].wood + weights_[ps.season].smola + weights_[ps.season].stone + weights_[ps.season].berry + weights_[ps.season].winPoints;

	for (int it = 0; it < resourceRecountIters; it++) {
		cardsInHandPlayValues = measureCardsToPlay(ps.hand);
		cardsInHandPotentials = measureCardsPotential(ps.hand);
		cardsInMeadowPlayValues = measureCardsToPlay(ge.getGs().meadow);
		cardsInMeadowPotentials = measureCardsPotential(ge.getGs().meadow);

		demand_ = ResourceWeights{};
		
		std::vector<std::pair<double, const Card*>> pretendents;
		pretendents.reserve(20);
		for (const auto& [value, idx]: cardsInHandPlayValues) {
			pretendents.emplace_back(value, ps.hand[idx]);
		}
		for (const auto& [value, idx]: cardsInMeadowPlayValues) {
			pretendents.emplace_back(value, ge.getGs().meadow[idx]);
		}

		if (!pretendents.empty()) {
			std::sort(pretendents.rbegin(), pretendents.rend());

			double norm = pretendents[0].first;
			double discount = 1.0;
			for (const auto& [value, card] : pretendents) {
				if (value <= 0) {
					continue;
				}
				demand_ += card->cost * (value / norm * discount);
				discount /= resourceWeightDiscount;
			}
		}

		demand_ -= ps.resources * 1.0;
		demand_.card += comfortableCardsInHand;

		// std::cout << "demand: " << demand_x100.to_string() << std::endl;
		preparedWeights_ = weights_[ps.season];
		preparedWeights_.wood = preparedWeights_.wood * pow(resourceBase, demand_.wood);
		preparedWeights_.smola = preparedWeights_.smola * pow(resourceBase, demand_.smola);
		preparedWeights_.stone = preparedWeights_.stone * pow(resourceBase, demand_.stone);
		preparedWeights_.berry = preparedWeights_.berry * pow(resourceBase, demand_.berry);
		preparedWeights_.winPoints = preparedWeights_.winPoints * ps.city.size() / occupiedCitySlotsTh;
		
		double norm = ((double)(preparedWeights_.wood + preparedWeights_.smola + preparedWeights_.stone + preparedWeights_.berry + preparedWeights_.winPoints)) / sum;
		preparedWeights_.wood /= norm;
		preparedWeights_.smola /= norm;
		preparedWeights_.berry /= norm;
		preparedWeights_.stone /= norm;
		preparedWeights_.winPoints /= norm;

		// preparedWeights.anyResource = std::max(preparedWeights.wood, std::max(preparedWeights.smola, std::max(preparedWeights.berry, preparedWeights.stone))) * anyResourceCoef;
		// std::cout << "weights: " << preparedWeights.to_string() << std::endl;
	}

	if (ps.handSize == ps.hand.size()) {
		preparedWeights_.card = 0;
	} else {
		preparedWeights_.card = weights_[ps.season].card * pow(resourceBase, demand_.card);
	}
	// double x = 1.0;
}

void SimpleBot::clearHandPrecalc() {
	cardsInHandPlayValues.clear();
	cardsInHandPotentials.clear();
}

void SimpleBot::evalHandPrecalc()
{
	const auto& ps = ge.getPlayer();
	cardsInHandPlayValues = measureCardsToPlay(ps.hand);
	cardsInHandPotentials = measureCardsPotential(ps.hand);
}

std::string SimpleBot::renderState() const {
	std::string ret;
	ret.reserve(1000);

	const auto& ps = ge.getPlayer();

	ret += "weithgs: " + preparedWeights_.to_string() + "<br>";
	ret += "demand: " + demand_.to_string() + "<br>";
	ret += "<details><summary>card values</summary>";
	auto vals = measureCardsInHandToPlay();
	ret += "Hand play:<br>";
	for (const auto& [val, idx] : vals) {
		ret += ps.hand[idx]->name + ": " + std::to_string(val) + "<br>";
	}
	ret += "<br>Hand potential:<br>";
	vals = measureCardsInHandPotential();
	for (const auto& [val, idx] : vals) {
		ret += ps.hand[idx]->name + ": " + std::to_string(val) + "<br>";
	}
	ret += "<br>Meadow play:<br>";
	vals = measureCardsInMeadowToPlay();
	for (const auto& [val, idx] : vals) {
		ret += ge.getGs().meadow[idx]->name + ": " + std::to_string(val) + "<br>";
	}
	ret += "<br>Meadow potential:<br>";
	vals = measureCardsInMeadowPotential();
	for (const auto& [val, idx] : vals) {
		ret += ge.getGs().meadow[idx]->name + ": " + std::to_string(val) + "<br>";
	}

	ret += "</details>";

	ret += "Quests card left: ";
	for (const auto& [l, idx]: enumerate(questCardsLeft[0])) {
		ret += std::to_string(l) + " ";
	}
	ret += "<br>";

	return ret;
}
