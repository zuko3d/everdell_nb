#include "PlayerState.h"

#include <cassert>

size_t countColoredCards(const PlayerState& ps, Color color) {
	size_t cnt = 0;
	for (const auto& c : ps.city) {
		if (c.cardInfo->color == color) {
			cnt++;
		}
	}
	return cnt;
}

std::array<size_t, 5> countColoredCards(const PlayerState& ps) {
	std::array<size_t, 5> cnt{ { 0 } };
	for (const auto& c : ps.city) {
		cnt[c.cardInfo->color]++;
	}
	return cnt;
}

void PlayerState::validate() const
{
	assert(resources.wood >= 0);
	assert(resources.smola >= 0);
	assert(resources.stone >= 0);
	assert(resources.berry >= 0);
	assert(resources.card == hand.size());
	assert(hand.size() <= handSize);

	int slots = 0;
	int attached = 0;
	for (const auto& c: city) {
		slots += c.cardInfo->occupiedCitySlots;
		if (c.hasAttachedCard) attached++;
	}
	assert(attached % 2 == 0);
	assert(freeCitySlots == 15 - slots + attached / 2);
	assert(freeCitySlots >= 0);
	assert(resources.anyResource == 0);
}

int PlayerState::greenProcsLeft() const {
	static int left[] = {2, 1, 1, 0};
	return left[season];
}

bool PlayerState::needSlotForCard(const Card* card) const {
	if (freeCitySlots >= card->occupiedCitySlots) {
		return false;
	}
	if (!card->attachableToName.empty()) {
		for (const auto& c: city) {
			if (!c.hasAttachedCard && c.cardInfo->idx == card->attachableToCard->idx) {
				return false;
			}
		}
	}
	return true;
}

HasSlot PlayerState::hasSlotForCard(const Card* card, DiscountType discount) const {
	// if (card->idx == Card::jokerCard->idx) {
	// 	return true;
	// }

	if (card->isUnique && hasCardInCity(card)) {
		return HasSlot::NoSlot;
	}

	if (freeCitySlots >= card->occupiedCitySlots) {
		return HasSlot::HasEmptySlot;
	}

	if (!card->attachableToName.empty()) {
		for (const auto& c: city) {
			if (!c.hasAttachedCard && c.cardInfo->idx == card->attachableToCard->idx) {
				return HasEmptySlot;
			}
		}
	}

	if (discount == DiscountType::NoneDiscount && (card->idx != Card::jokerCard->idx)) {
		if (card->isBuilding) {
			if (availableDiscounts[DiscountType::Crane]) {
				return HasSlot::OnlyWithSlotSac;
			}
		} else {
			if (availableDiscounts[DiscountType::Traktirshik]) {
				return HasSlot::OnlyWithSlotSac;
			}
		}
		if ((availableDiscounts[DiscountType::Dungeon] || availableDiscounts[DiscountType::Inventor]) && (card->cost.cmc() >= 4.0)) {
			return HasSlot::OnlyWithSlotSac;
		}
	}

	return HasSlot::NoSlot;
}

int PlayerState::countCardInCity(const Card* card) const
{
	return cityLookup.at(card->idx);
}

bool PlayerState::hasCardInCity(const Card* card) const
{
	return countCardInCity(card) > 0;
}

const CardInCity* PlayerState::getCardInCity(const Card* card) const {
	for (const auto& c: city) {
		if (c.cardInfo->idx == card->idx) {
			return &c;
		}
	}

	return nullptr;
}

DiscountType PlayerState::chooseBestDiscount(const Card& card) const
{
	if (needSlotForCard(&card)) {
		if (card.isBuilding) {
			if (availableDiscounts[DiscountType::Crane]) {
				return DiscountType::Crane;
			}
		} else {
			if (availableDiscounts[DiscountType::Traktirshik]) {
				return DiscountType::Traktirshik;
			}
		}
		if (availableDiscounts[DiscountType::Dungeon]) {
			return DiscountType::Dungeon;
		}
		if (availableDiscounts[DiscountType::Inventor]) {
			return DiscountType::Inventor;
		}
	}
	if (card.isBuilding) {
		const auto cmc = card.cost.cmc();
		if (availableDiscounts[DiscountType::Crane] && (card.cost.cmc() >= 3.2)) {
			return DiscountType::Crane;
		} else if (cmc >= 4.0) {
			if (availableDiscounts[DiscountType::Inventor]) {
				return DiscountType::Inventor;
			}

			if (availableDiscounts[DiscountType::Dungeon]) {
				if (coloredCardsLookup[Color::Grey] > 0 || city.size() > 4) {
					if (std::find_if(city.begin(), city.end(), [&](const auto& c) { return !c.cardInfo->isBuilding; }) != city.end()) {
						return DiscountType::Dungeon;
					}
				}
			}
			
		}
	} else {
		if (availableDiscounts[DiscountType::Traktirshik] && (card.cost.berry >= 3)) {
			return DiscountType::Traktirshik;
		}
		if (card.cost.berry >= 3 && availableDiscounts[DiscountType::Inventor]) {
			return DiscountType::Inventor;
		}
	}

	if (availableDiscounts[DiscountType::Judge]) {
		return DiscountType::Judge;
	}

	return DiscountType::NoneDiscount;
}

Resource PlayerState::applyDiscount(DiscountType discount, const Card& card) const
{
	Resource ret = card.cost;

	if (discount == DiscountType::ForFree) {
		return {};
	}
	if (discount == NoneDiscount) {
		return ret;
	}

	if (discount == DiscountType::Judge) {
		assert(hasCardInCity(Card::judgeCard));
		int d = 0;
		if (resources.wood > ret.wood) {
			ret.wood++;
			d++;
		}
		else if (resources.smola > ret.smola) {
			ret.smola++;
			d++;
		}
		else if (resources.berry > ret.berry) {
			ret.berry++;
			d++;
		}
		if (d > 0) {
			ret.removeMostValueable(resources);
		}
	}
	else if (discount != DiscountType::NoneDiscount) {
		for (int i = 0; i < 3; i++) {
			ret.removeMostValueable(resources);
		}
	}
	return ret;
}

int PlayerState::canPlayCreatureForShield(const Card* card) const
{
	assert(hasSlotForCard(card));
	if (card->mainBuildingCard != nullptr) {
		for (const auto& [c, idx]: enumerate(city)) {
			if (c.cardInfo->idx == card->mainBuildingCard->idx && !c.shieldUsed) {
				return idx;
			}
		}
	}
	for (const auto& [c, idx] : enumerate(city)) {
		if (!c.shieldUsed && c.cardInfo->canHaveResident(card)) {
			return idx;
		}
	}

	return -1;
}

Resource PlayerState::minimalPrice(const Card* card) const
{
	if (canPlayCreatureForShield(card) > -1) {
		return Resource{};
	}

	return applyDiscount(chooseBestDiscount(*card), *card);
}
