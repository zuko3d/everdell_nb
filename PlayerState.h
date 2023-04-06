#pragma once

#include <array>
#include <vector>

#include "Quest.h"
#include "Resource.h"
#include "Card.h"
#include "Utils.h"

enum CanPlay {
	CantPlay = 0,
	JustPlay = 1,
	OnlyForShield = 2,
	OnlyWithSac = 4,
};

enum HasSlot {
	NoSlot = 0,
	HasEmptySlot = 1,
	OnlyWithSlotSac = 2,
};

struct CardInCity {
	CardInCity(const Card* cardInfo_)
		: cardInfo(cardInfo_)
	{ }

	const Card* cardInfo;
	bool shieldUsed = false;
	bool occupiedByWorker = false;
	bool hasAttachedCard = false;
	Resource resources;
};

struct PlayerState
{
	std::array<bool, DiscountType::NoneDiscount> availableDiscounts;
	std::array<bool, TriggerType::NoneTrigger> availableTriggers;

	int freeWorkers = 2;
	// std::vector<int> deployedWorkersPositions;
	std::vector<const Card*> hand;
	std::vector<CardInCity> city;
	std::vector<Quest> finishedQuests;
	int freeCitySlots = 15;
	int goldShields = 3;
	Resource resources;
	size_t handSize = 8; // bank increases it!
	int season = 0;
	bool passed = false;

	std::vector<char> cityLookup = std::vector<char>(100, 0);
	std::array<int, 5> coloredCardsLookup{{0}};

	void validate() const;
	int greenProcsLeft() const;

	HasSlot hasSlotForCard(const Card* card, DiscountType discount = DiscountType::NoneDiscount) const;

	CanPlay canPlayCard(const Card* card, DiscountType discount = DiscountType::NoneDiscount) const {
		int ret = JustPlay;
		const auto hasSlot = hasSlotForCard(card);
		if (hasSlot == NoSlot) {
			return CantPlay;
		}
		if (hasSlot == HasSlot::OnlyWithSlotSac) {
			ret |= CanPlay::OnlyWithSac;
			if (discount != NoneDiscount) {
				return CantPlay;
			}
		}
		Resource actualPrice = applyDiscount(discount, *card);

		if (resources >= actualPrice) {
			return (CanPlay) ret;
		} else {
			if (discount == NoneDiscount) {
				actualPrice = applyDiscount(chooseBestDiscount(*card), *card);
				if (resources >= actualPrice) {
					return (CanPlay) ret;
				} else {
					if (canPlayCreatureForShield(card) > -1) {
						ret |= OnlyForShield;
						return (CanPlay) ret;
					} else {
						return CantPlay;
					}
				}
			} else {
				return CantPlay;
			}
		}

		return CantPlay;
	}

	bool needSlotForCard(const Card* card) const;

	int countCardInCity(const Card* card) const;
	bool hasCardInCity(const Card* card) const;
	const CardInCity* getCardInCity(const Card* card) const;

	DiscountType chooseBestDiscount(const Card& card) const;
	Resource applyDiscount(DiscountType discount, const Card& card) const;

	int canPlayCreatureForShield(const Card* card) const;

	Resource minimalPrice(const Card* card) const;
};
