#include "Card.h"

#include <algorithm>
#include <cassert>

#include "GameEngine.h"
#include "Utils.h"

const Card* Card::husbandCard;
const Card* Card::wifeCard;
const Card* Card::traktirCard;
const Card* Card::bardCard;
const Card* Card::hightowerCard;
const Card* Card::zemlekopCard;
const Card* Card::courtCard;
const Card* Card::judgeCard;
const Card* Card::healerCard;
const Card* Card::pigeonCard;
const Card* Card::rangerCard;
const Card* Card::dvornikCard;
const Card* Card::towerCard;
const Card* Card::historicCard;
const Card* Card::razvaliniCard;
const Card* Card::lavochnikCard;
const Card* Card::postCard;
const Card* Card::graveyardCard;
const Card* Card::shepherdCard;
const Card* Card::monkCard;
const Card* Card::dungeonCard;
const Card* Card::rezchikCard;
const Card* Card::chapelCard;
const Card* Card::grobovshikCard;
const Card* Card::vetochnikCard;
const Card* Card::teacherCard;
const Card* Card::univerCard;
const Card* Card::queenCard;
const Card* Card::craneCard;
const Card* Card::inventorCard;
const Card* Card::traktirshikCard;
const Card* Card::skladCard;
const Card* Card::farmCard = nullptr;
const Card* Card::jokerCard = nullptr;

std::unordered_map<std::string, const Card*> Card::cardByName;
std::array<CardEffect, TriggerType::NoneTrigger> Card::allTriggers;

int noPoints(const std::vector<int>&) {
	return 0;
}

Resource noResources(const std::vector<int>&) {
	return {};
}

std::vector<Card> Card::genAllCards()
{
	std::vector<Card> ret;

	assert(Card::farmCard == nullptr);
	
	// трактир неоптимален (не учитывает свою скидку)

	ret.push_back(Card{
		.name = "Архитектор",
		.mainBuildingName = "Кран",
		.purpleValue = [](const Card* card) { return 0; },
		.color = Color::Purple,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 4},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.awardWp(ge.getPlayer().resources.smola + ge.getPlayer().resources.stone);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.winPoints = 2 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Кран",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.registerDiscount(DiscountType::Crane);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.anyResource = 3}; },
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{ }; },
		.idx = (int)ret.size(),
	});
	
	ret.push_back(Card{
		.name = "Бард",
		.mainBuildingName = "Театр",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 0,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			const_cast<SimpleBot*>(ge.getBot())->evalHandPrecalc();
			int discarded = ge.chooseAndDiscard(5);
			ge.awardWp(discarded);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.card = -5, .winPoints = 5}; },
		.idx = (int)ret.size(),
	});
	ret.push_back(Card{
		.name = "Театр",
		.mainBuildingName = "",
		.purpleValue = [](const Card* card) { return !card->isBuilding && card->isUnique; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 3,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 3, .smola = 1, .stone = 1},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			ge.awardWp(std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return !c.cardInfo->isBuilding && c.cardInfo->isUnique;
			}));
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			return Resource{.winPoints = ge.getPlayer().freeCitySlots / 4 + (int) std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return !c.cardInfo->isBuilding && c.cardInfo->isUnique;
			})};
		},
		.idx = (int)ret.size(),
	});


	ret.push_back(Card{
		.name = "Веточник",
		.mainBuildingName = "Ветковозка",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.awardResources(Resource{.wood = 2 * countInCityByName(ge.getPlayer(), "Ферма") });
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.wood = 2 * countInCityByName(ge.getPlayer(), "Ферма") };
		},
		.imagineSynergyBonus = [](const GameEngine& ge) { return Resource{.wood = 2 }; },
		.idx = (int)ret.size(),
	});
	ret.push_back(Card{
		.name = "Ветковозка",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 1, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) { ge.awardResources({.wood = 2}); },
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.wood = 2 }; },
		.idx = (int)ret.size(),
	});
	ret.push_back(Card{
		.name = "Дворник",
		.mainBuildingName = "Смоловарня",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 1,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 3},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			const CardInCity* bestRet = nullptr;
			double bestPts = -1;
			const auto& ps = ge.getPlayer();
			for (const auto& c : ps.city) {
				if (c.cardInfo->color == Color::Green) {
					if (c.cardInfo->idx == zemlekopCard->idx) {
						if (ge.isCardInEffectStack(zemlekopCard)) {
							continue;
						}
					}
					if (c.cardInfo->name != "Дворник") {
						const auto effect = c.cardInfo->imagineProcEffect(ge);
						const auto pts = ge.getBot()->measureResourcesToGain(effect);
						if (pts > bestPts) {
							bestPts = pts;
							bestRet = &c;
						}
					}
				}
			}
			if (bestRet != nullptr) {
				ge.log("copy effect", bestRet->cardInfo);
				ge.procEffect(const_cast<CardInCity*>(bestRet));
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) { 
			Resource bestRet;
			double bestPts = -1;
			const auto& ps = ge.getPlayer();
			for (const auto& c : ps.city) {
				if (c.cardInfo->color == Color::Green) {
					if (c.cardInfo->name != "Дворник") {
						const auto effect = c.cardInfo->imagineProcEffect(ge);
						const auto pts = ge.getBot()->measureResourcesToGain(effect);
						if (pts > bestPts) {
							bestPts = pts;
							bestRet = effect;
						}
					}
				}
			}
			return bestRet;
		},
		.idx = (int)ret.size(),
	});
	ret.push_back(Card{
		.name = "Смоловарня",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.smola = 1, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) { ge.awardResources(Resource{.smola = 1 }); },
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.smola = 1 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Лекарь",
		.mainBuildingName = "Университет",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 3,
		.winPoints = 4,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 4},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			int amount = std::min(3, ge.getPlayer().resources.berry);
			ge.awardResources(Resource{.berry = -amount, .winPoints = amount });
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.berry = -3, .winPoints = 3};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Университет",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.winPoints = 3,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.smola = 1, .stone = 2},
		.amountInGame = 2,
		// .effect = [](GameEngine& ge) {},
		.deployEffect = [](GameEngine& ge) {
			ge.chooseAndSacFromCity(1, false);
			ge.awardResources(ge.getGs().graveyard.back()->cost + Resource{.anyResource = 1});
			ge.awardWp(1);
		},
		// .imagineProcEffect = [](const GameEngine& ge) {
		// 	return Resource{.anyResource = 2, .winPoints = 1};
		// },
		.imagineDeployEffect = [](const GameEngine& ge) {
			const auto measures = ge.getBot()->measureCardsInCityToSac(true, univerCard->idx);
			auto idx = std::distance(measures.begin(), std::min_element(measures.begin(), measures.end()));
			if (measures.empty() || measures[idx] >= 0) {
				return Resource{};
			} else {
				return ge.getPlayer().city[idx].cardInfo->cost + Resource{ .anyResource = 1, .winPoints = 1 };
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.anyResource = 3, .winPoints = 1 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Шут",
		.mainBuildingName = "Ярмарка",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = -2,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			assert(ge.getPlayer().city.back().cardInfo->idx == jokerCard->idx);
			ge.awardCardFromCity(ge.nextPlayer(), ge.getPlayer().city.size() - 1);
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			if (ge.getPlayer(ge.nextPlayer()).hasSlotForCard(jokerCard)) {
				return Resource{ .winPoints = 5 };
			} else {
				// never play or game will crash
				return Resource{.winPoints = -500 };
			}			
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Ярмарка",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 3,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 1, .smola = 2, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.drawFromDeck(2);
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.card = 2};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Историк",
		.mainBuildingName = "Башня",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.registerTrigger(TriggerType::Historic);
		},
		.imagineProcEffect = [](const GameEngine& ge)
		{
			return Resource{};
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season)
		{
			if (season == 1 || season == 3)
			{
				return Resource{ .card = 1 + ge.getPlayer().freeCitySlots / 4 };
			} else {
				return Resource{.card = 1 + ge.getPlayer().freeCitySlots / 2 };
			}
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Башня",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 0,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 3, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.getCOI()->resources.winPoints = 3;
			ge.registerTrigger(TriggerType::Tower);
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{ };
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{.wood = 1, .smola = 1, .berry = 1}; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Муж",
		.mainBuildingName = "Ферма",
		.attachableToName = "Жена",
		.color = Color::Green,
		.greenProcOrder = 2,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 3},
		.amountInGame = 4,
		.effect = [](GameEngine& ge) {
			if (ge.getPlayer().hasCardInCity(Card::farmCard) && ge.getCOI()->hasAttachedCard) {
				ge.awardResources(ge.getBot()->chooseResourcesToGain(1));
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			if (ge.getPlayer().hasCardInCity(Card::farmCard) && ge.getPlayer().countCardInCity(husbandCard) > ge.getPlayer().countCardInCity(husbandCard)) {
				return Resource{.anyResource = 1};
			} else {
				return Resource{.anyResource = 0};
			}
		},
		.imagineSynergyBonus = [](const GameEngine& ge) { return Resource{.anyResource = 1}; },
		.idx = (int)ret.size(),
	});
	husbandCard = &ret.back();

	ret.push_back(Card{
		.name = "Жена",
		.mainBuildingName = "Ферма",
		.purpleValue = [](const Card* card) { if (card->idx == husbandCard->idx) return 3; return 0; },
		.attachableToName = "Муж",
		.color = Color::Purple,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 3},
		.amountInGame = 4,
		.effect = [](GameEngine& ge) {
			if (ge.getCOI()->hasAttachedCard) {
				ge.awardWp(3);
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.winPoints = 3};
		},
		.idx = (int)ret.size(),
	});
	wifeCard = &ret.back();

	ret.push_back(Card{
		.name = "Вечное Древо",
		.mainBuildingName = "",
		.canHaveResident = [](const Card* card) { return true; },
		.purpleValue = [](const Card* card) { return card->color == Color::Purple; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 5,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 3, .smola = 3, .stone = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.awardWp(ge.getPlayer().coloredCardsLookup[Color::Purple]);
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			return Resource{.berry = 3, .winPoints = ge.getPlayer().freeCitySlots / 4 + (int)std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return c.cardInfo->color == Color::Purple;
			})};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Ферма",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 2, .smola = 1},
		.amountInGame = 8,
		.effect = [](GameEngine& ge) {
			ge.awardResources({.berry = 1});
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.berry = 1};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Трактирщик",
		.mainBuildingName = "Трактир",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.registerDiscount(DiscountType::Traktirshik);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.anyResource = 3}; },
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{ }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Трактир",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.oppCanJoinForPoints = 1,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 2, .smola = 1},
		.amountInGame = 3,
		// .effect = [](GameEngine& ge) {},
		.deployEffect = [](GameEngine& ge) {
			const auto bot = ge.getBot();
			const auto& ps = ge.getPlayer();
			if (ps.freeCitySlots > 0) {
				std::vector<const Card*> cards;
				std::vector<size_t> origin;
				Resource resources = ps.resources;
				resources.anyResource = 3;
				const auto& measures = bot->measureCardsInMeadowToPlay();
				for (int i = 7; i >= 0; i--) {
					const auto card = ge.getGs().meadow[measures[i].second];
					if (ps.canPlayCard(card, DiscountType::Traktir)) {
						ge.playCardFromMeadow(measures[i].second, DiscountType::Traktir);
						return;
					}
				}				
			}
		},
		// .imagineProcEffect = [](const GameEngine& ge) { return Resource{.anyResource = 1}; },
		.imagineDeployEffect = [](const GameEngine& ge)
		{
			if (ge.getPlayer().freeCitySlots > 0) {
				return Resource{.anyResource = 3};
			} else {
				return Resource{};
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.anyResource = 3};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Судья",
		.mainBuildingName = "Суд",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.registerDiscount(DiscountType::Judge);
		},
		.imagineProcEffect = [](const GameEngine& ge) { 
			return Resource{.anyResource = ge.getPlayer().freeCitySlots / 4};
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{ }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Суд",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 1, .smola = 1, .stone = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.registerTrigger(TriggerType::Court);
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.anyResource = ge.getPlayer().freeCitySlots / 2};
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{ }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Король",
		.mainBuildingName = "Замок",
		.purpleValue = [](const Card* card) { return 0; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 4,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 6},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			for (const auto& q: ge.getPlayer().finishedQuests) {
				if (q.isBasic) {
					ge.awardWp(1);
				} else {
					ge.awardWp(2);
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.winPoints = (int) ge.getPlayer().finishedQuests.size()}; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Замок",
		.mainBuildingName = "",
		.purpleValue = [](const Card* card) { return card->isBuilding && !card->isUnique; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 4,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 2, .smola = 3, .stone = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			for (const auto& c : ge.getPlayer().city) {
				if (c.cardInfo->isBuilding && !c.cardInfo->isUnique) {
					ge.awardWp(1);
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			return Resource{.winPoints = ge.getPlayer().freeCitySlots / 4 + (int)std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return c.cardInfo->isBuilding && !c.cardInfo->isUnique;
			})};
		},
		.idx = (int)ret.size(),
	});

	// яя

	ret.push_back(Card{
		.name = "Шахта",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 1, .smola = 1, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.awardResources({.stone = 1});
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.stone = 1}; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Землекоп",
		.mainBuildingName = "Шахта",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 1,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 3},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			CardInCity* bestRet = nullptr;
			double bestPts = -1;
			const int opp = ge.nextPlayer();
			const auto& oppPs = ge.getPlayer(opp);
			const auto& ps = ge.getPlayer();
			for (const auto& c : oppPs.city) {
				if (c.cardInfo->color == Color::Green) {
					if (c.cardInfo->name != "Дворник" && c.cardInfo->name != "Землекоп" && c.cardInfo->name != "Склад") {
						const auto effect = ge.imagineProcEffect(&c, opp);
						const auto pts = ge.getBot()->measureResourcesToGain(effect);
						if (pts > bestPts) {
							bestPts = pts;
							bestRet = const_cast<CardInCity*>(&c);
						}
					}
				}
			}
			if (bestRet != nullptr) {
				ge.log("copy effect", bestRet->cardInfo);
				ge.procEffect(bestRet);
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			Resource bestRet;
			double bestPts = -1;
			const int opp = ge.nextPlayer();
			const auto& oppPs = ge.getPlayer(opp);
			const auto& ps = ge.getPlayer();
			for (const auto& c : oppPs.city) {
				if (c.cardInfo->color == Color::Green) {
					if (c.cardInfo->name != "Дворник" && c.cardInfo->name != "Землекоп" && c.cardInfo->name != "Склад") {
						const auto effect = ge.imagineProcEffect(&c, opp);
						const auto pts = ge.getBot()->measureResourcesToGain(effect);
						if (pts > bestPts) {
							bestPts = pts;
							bestRet = effect;
						}
					}
				}
			}
			return bestRet;
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Монах",
		.mainBuildingName = "Монастырь",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 3,
		.winPoints = 0,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 1},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			int amount = std::min(2, ge.getPlayer().resources.berry);
			ge.awardResources({.berry = -amount, .winPoints = 2 * amount});
			ge.awardResources(ge.nextPlayer(), { .berry = amount });
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.berry = -2, .winPoints = 2}; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Монастырь",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 1, .smola = 1, .stone = 1},
		.amountInGame = 2,
		// .effect = [](GameEngine& ge) {},
		.deployEffect = [](GameEngine& ge) {
			const auto toSpend = ge.getBot()->chooseResourcesToPay(2);
			ge.spendResources(toSpend);
			ge.awardWp(4);
			ge.awardResources(ge.nextPlayer(), toSpend);
		},
		// .imagineProcEffect = [](const GameEngine& ge) { },
		.imagineDeployEffect = [](const GameEngine& ge)
		{
			const auto& res = ge.getPlayer().resources;
			if (res.wood + res.smola + res.stone + res.berry >= 2) {
				return ge.getBot()->chooseResourcesToPay(2) * -2 + Resource{ .winPoints = 4 };
			} else {
				return Resource{ };
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.anyResource = -3, .winPoints = 4 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Барахольщик",
		.mainBuildingName = "Развалины",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 2,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			const auto bot = ge.getBot();
			const auto& ps = ge.getPlayer();
			const int res = std::min(2, ps.resources.wood + ps.resources.smola + ps.resources.stone + ps.resources.berry);
			if (res > 0) {
				ge.awardResources(bot->chooseResourcesToGain(res) - bot->chooseResourcesToPay(res));
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& ps = ge.getPlayer();
			const int res = std::min(2, ps.resources.wood + ps.resources.smola + ps.resources.stone + ps.resources.berry);
			return Resource{.wood = -res, .anyResource = res};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Развалины",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 0,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{ },
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			if (ge.getPlayer().city.size() > 0) {
				ge.chooseAndSacFromCity(1, false, true);
				// assert(ge.getGs().graveyard.back()->idx != Card::razvaliniCard->idx);
				ge.awardResources(ge.getGs().graveyard.back()->cost);
				ge.drawFromDeck(2);
			}
		},
		.imagineProcEffect = [](const GameEngine& ge)
		{
			const auto measures = ge.getBot()->measureCardsInCityToSac();
			auto order = argsort(measures);
			const auto& ps = ge.getPlayer();
			for (size_t i = 0; i < order.size(); i++) {
				size_t idx = order[i];
				const auto& cardInCity = ps.city[idx];
				if (cardInCity.cardInfo->isBuilding && cardInCity.cardInfo->idx != razvaliniCard->idx) {
					if (measures[idx] < 0) {
						return cardInCity.cardInfo->cost + Resource{ .card = 2 };
					} else {
						// just don't play
						return Resource{ .winPoints = -500 };
					}
				}
				
			}
			return Resource{ .winPoints = -500 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Почтальон",
		.mainBuildingName = "Почта",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 0,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{ .berry = 2 },
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			const auto& ps = ge.getPlayer();
			std::vector<const Card*> cards;
			cards.reserve(2);
			for (int i = 0; i < 2; i++) {
				const auto topDeck = ge.getGs().deck.back();
				if (topDeck->winPoints <= 3 && ps.canPlayCard(topDeck, DiscountType::ForFree)) {
					cards.push_back(topDeck);
				}
				ge.popDeck();
			}

			if (!cards.empty()) {
				const auto values = ge.getBot()->measureCardsToPlay(cards, DiscountType::ForFree);
				if (values.back().first > 0) {
					const int idx = values.back().second;
					ge.playCard(*cards[idx], DiscountType::ForFree);
					if (cards.size() > 1) {
						ge.putToGraveyard(cards[1 - idx]);
					}
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			if (ge.getPlayer().freeCitySlots > 1) {
				return Resource{.anyResource = 2, .winPoints = 1};
			} else {
				return Resource{};
			}
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Почта",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.oppCanJoinForPoints = 1,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 1, .smola = 2 },
		.amountInGame = 3,
		// .effect = [](GameEngine& ge) { },
		.deployEffect = [](GameEngine& ge)
		{
			ge.chooseAndAwardCardsToOpp(2);
			ge.drawFromDeck(ge.getPlayer().handSize - ge.getPlayer().hand.size());
		},
		// .imagineProcEffect = [](const GameEngine& ge) { return Resource{.card = 1}; },
		.imagineDeployEffect = [](const GameEngine& ge) { return Resource{.card = (int)(ge.getPlayer().handSize - ge.getPlayer().hand.size())}; },
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.card = 4 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Королева",
		.mainBuildingName = "Дворец",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.winPoints = 4,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 5 },
		.amountInGame = 2,
		.deployEffect = [](GameEngine& ge) {
			if (ge.getPlayer().freeCitySlots <= 0) {
				return;
			}
			std::vector<const Card*> cards;
			std::vector<int> origin;
			const auto& ps = ge.getPlayer();
			cards.reserve(16);
			origin.reserve(16);
			for (const auto& [c, idx]: enumerate(ge.getGs().meadow)) {
				if (c->winPoints <= 3 && ps.canPlayCard(c, DiscountType::ForFree)) {
					cards.push_back(c);
					origin.push_back(-(int) idx - 1);
				}
			}
			for (const auto& [c, idx] : enumerate(ge.getPlayer().hand)) {
				if (c->winPoints <= 3 && ps.canPlayCard(c, DiscountType::ForFree)) {
					cards.push_back(c);
					origin.push_back(idx + 1);
				}
			}

			if (!cards.empty())
			{	
				const auto idx = ge.getBot()->chooseBestCardToPlay(cards, DiscountType::ForFree);
				if (origin[idx] < 0) {
					ge.playCardFromMeadow(-origin[idx] - 1, DiscountType::ForFree);
				} else {
					ge.playCardFromHand(origin[idx] - 1, DiscountType::ForFree);
				}
			}
		},
		// .imagineProcEffect = [](const GameEngine& ge)
		// {
		// 	if (ge.getPlayer().freeCitySlots > 0) {
		// 		return Resource{.anyResource = 2, .winPoints = 1};
		// 	} else {
		// 		return Resource{};
		// 	}
		// },
		.imagineDeployEffect = [](const GameEngine& ge)
		{
			if (ge.getPlayer().freeCitySlots > 0) {
				return Resource{.anyResource = 2, .winPoints = 1};
			} else {
			    return Resource{};
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.anyResource = 2, .winPoints = 1 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Дворец",
		.mainBuildingName = "",
		.purpleValue = [](const Card* card) { return card->isBuilding && card->isUnique; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 4,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 2, .smola = 3, .stone = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			for (const auto& c : ge.getPlayer().city) {
				if (c.cardInfo->isBuilding && c.cardInfo->isUnique) {
					ge.awardWp(1);
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			return Resource{.winPoints = ge.getPlayer().freeCitySlots / 4 + (int)std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return c.cardInfo->isBuilding && c.cardInfo->isUnique;
			})};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Следопыт",
		.mainBuildingName = "Темница",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			Resource bestProfit;
			double bestScore = 0;
			const auto bot = ge.getBot();
			auto& ps = ge.getPlayer();
			int bestIdx = -1;

			for (const auto& [fa, idx] : enumerate(ps.city)) {
				if (fa.occupiedByWorker) {
					double pts = bot->measureResourcesToGain(fa.cardInfo->imagineDeployEffect(ge));
					if (pts > bestScore) {
						bestScore = pts;
						bestIdx = idx;
					}
				}
			}

			if (bestIdx >= 0) {
				ge.log("ranger moved worker from", ps.city[bestIdx].cardInfo);
				const_cast<PlayerState&>(ps).city[bestIdx].occupiedByWorker = false;
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			Resource bestProfit;
			double bestScore = 0;
			const auto bot = ge.getBot();
			auto& ps = ge.getPlayer();

			for (const auto& [fa, idx] : enumerate(ge.getGs().forestActions)) {
				if (ps.resources >= fa.price) {
					double pts = bot->measureResourcesToGain(fa.profit - fa.price);
					if (pts > bestScore)
					{
						bestScore = pts;
						bestProfit = fa.profit - fa.price;
					}
				}
			}

			return bestProfit;
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Темница",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 0,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.smola = 1, .stone = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.registerDiscount(DiscountType::Dungeon);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.anyResource = 3}; },
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{ }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Пастырь",
		.mainBuildingName = "Часовня",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 3},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.awardResources({ .berry = 3 });
			auto chapel = ge.getPlayer().getCardInCity(chapelCard);
			if (chapel != nullptr) {
				ge.awardResources(chapel->resources);
			} else {
				ge.awardResources(ge.nextPlayer(), { .berry = 3 });
			}
		},
		.imagineProcEffect = [](const GameEngine& ge)
		{
			auto chapel = ge.getPlayer().getCardInCity(chapelCard);
			if (chapel != nullptr) {
				return chapel->resources;
			} else {
				return Resource{};
			}
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Часовня",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 2, .smola = 1, .stone = 1},
		.amountInGame = 2,
		// .effect = [](GameEngine& ge) {},
		.deployEffect = [](GameEngine& ge) {
			auto coi = ge.getCOI();
			coi->resources.winPoints++;
			ge.drawFromDeck(2 * coi->resources.winPoints);
		},
		// .imagineProcEffect = [](const GameEngine& ge) { return Resource{ }; },
		.imagineDeployEffect = [](const GameEngine& ge)
		{
			const auto chapel = ge.getPlayer().getCardInCity(chapelCard);
			if (chapel != nullptr) {
				return Resource{ .card = 2 * (chapel->resources.winPoints + 1), .winPoints = 1 };
			} else {
				return Resource{ .card = 2, .winPoints = 1 };
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			return Resource{.card = 2, .winPoints = 1 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Лавочник",
		.mainBuildingName = "Лавка",
		.attachableToName = "",
		.color = Color::Blue,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			ge.registerTrigger(TriggerType::Lavochnik);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{ }; },
		.imagineLaterEffect = [](const GameEngine& ge, int season)
		{
			if (season == 1 || season == 3)
			{
				return Resource{.berry = 1 + ge.getPlayer().freeCitySlots / 6 };
			} else {
				return Resource{.berry = 1 + ge.getPlayer().freeCitySlots / 3 };
			}
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Лавка",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 0,
		.winPoints = 1,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.smola = 1, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			if (ge.getPlayer().hasCardInCity(farmCard)) {
				ge.awardResources({ .berry = 2 });
			} else {
				ge.awardResources({.berry = 1 });
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.berry = 1 + ge.getPlayer().hasCardInCity(farmCard) }; },
		.imagineSynergyBonus = [](const GameEngine& ge) { return Resource{.berry = 1 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Учитель",
		.mainBuildingName = "Школа",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 1,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge)
		{
			std::vector<const Card*> cards;
			auto& ps = ge.getPlayer();
			const int amount = std::min(2ul, ps.handSize - ps.hand.size());

			if (amount > 0) {
				if (amount == 1) {
					ge.drawFromDeck(1);
				} else {
					for (int i = 0; i < amount; i++)
					{
						cards.push_back(ge.getGs().deck.back());
						ge.popDeck();
					}

					const int idx = ge.getBot()->measureCardsPotential(cards).back().second;
					ge.awardCard(cards[idx]);
					if (ge.getPlayer(ge.nextPlayer()).hand.size() < ge.getPlayer(ge.nextPlayer()).handSize) {
						ge.awardCard(ge.nextPlayer(), cards[1 - idx]);
					} else {
						ge.putToGraveyard(cards[1 - idx]);
					}
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			return Resource{.card = 1 };
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Школа",
		.mainBuildingName = "",
		.purpleValue = [](const Card* card) { return !card->isBuilding && !card->isUnique; },
		.attachableToName = "",
		.color = Color::Purple,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 2, .smola = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			for (const auto& c : ge.getPlayer().city) {
				if (!c.cardInfo->isBuilding && !c.cardInfo->isUnique) {
					ge.awardWp(1);
				}
			}
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			const auto& city = ge.getPlayer().city;
			return Resource{.winPoints = ge.getPlayer().freeCitySlots / 3 + (int)std::count_if(city.begin(), city.end(), [&](const auto& c) {
				return !c.cardInfo->isBuilding && !c.cardInfo->isUnique;
			})};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Гробовщик",
		.mainBuildingName = "Кладбище",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 1,
		.isBuilding = false,
		.isUnique = true,
		.cost = Resource{.berry = 2},
		.amountInGame = 2,
		.effect = [](GameEngine& ge) {
			const auto bot = ge.getBot();
			auto& ps = ge.getPlayer();
			const auto values = bot->measureCardsInMeadowToPlay();

			for (int i = 0; i < 3; i++) {
				ge.dropCardFromMeadow(values[i].second);
			}

			ge.chooseAndDrawFromMeadow(1);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{ .card = 1 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Кладбище",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.winPoints = 0,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.stone = 2},
		.amountInGame = 2,
		// .effect = [](GameEngine& ge) { },
		.deployEffect = [](GameEngine& ge)
		{
			const auto& ps = ge.getPlayer();
			std::vector<const Card*> cards;
			cards.reserve(4);
			const auto sz = std::min(4ul, ge.getGs().graveyard.size());
			for (int i = 0; i < sz; i++) {
				const auto topDeck = ge.getGs().graveyard[ge.getGs().graveyard.size() - i - 1];
				if (ps.canPlayCard(topDeck)) {
					cards.push_back(topDeck);
				}
			}

			if (!cards.empty()) {
				if (cards.size() > 1) {
					const auto idx = ge.getBot()->chooseBestCardToPlay(cards, DiscountType::ForFree);
					ge.playCard(*cards[idx], DiscountType::ForFree);
				} else {
				  ge.playCard(*cards[0], DiscountType::ForFree);
				}
			}
		},
		// .imagineProcEffect = [](const GameEngine& ge) { return Resource{ }; },
		.imagineDeployEffect = [](const GameEngine& ge)
		{
			if (ge.getPlayer().freeCitySlots > 0) {
				return Resource{.winPoints = 3 };
			} else {
				return Resource{ };
			}
		},
		.imagineLaterEffect = [](const GameEngine& ge, int season) {
			if (ge.getPlayer().freeCitySlots > 0) {
				return Resource{.winPoints = 3 };
			} else {
			    return Resource{ };
			}
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Странник",
		.mainBuildingName = "Вышка",
		.attachableToName = "",
		.color = Color::Grey,
		.winPoints = 1,
		.occupiedCitySlots = 0,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			ge.drawFromDeck(3);
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.card = 3 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Вышка",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Red,
		.deployable = true,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = true,
		.cost = Resource{.wood = 1, .smola = 1, .stone = 1},
		.amountInGame = 3,
		// .effect = [](GameEngine& ge) {},
		.deployEffect = [](GameEngine& ge) {
			Resource bestProfit;
			double bestScore = 0;
			const auto bot = ge.getBot();
			auto& ps = ge.getPlayer();

			for (const auto& [fa, idx]: enumerate(ge.getGs().forestActions)) {
				if (ps.resources >= fa.price) {
					double pts = bot->measureResourcesToGain(fa.profit - fa.price);
					if (pts > bestScore)
					{
						bestScore = pts;
						bestProfit = fa.profit - fa.price;
					}
				}
			}
			for (const auto& [fa, idx] : enumerate(ge.getGs().baseActions)) {
				if (ps.resources >= fa.price) {
					double pts = bot->measureResourcesToGain(fa.profit - fa.price);
					if (pts > bestScore)
					{
						bestScore = pts;
						bestProfit = fa.profit - fa.price;
					}
				}
			}
			
			ge.awardResources(bestProfit);
		},
		// .imagineProcEffect = [](const GameEngine& ge) { return Resource{.card = 1 }; },
		.imagineDeployEffect = [](const GameEngine& ge) { return Resource{.anyResource = 2 }; },
		.imagineLaterEffect = [](const GameEngine& ge, int season) { return Resource{.anyResource = 2 }; },
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Резчик",
		.mainBuildingName = "Склад",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 3,
		.winPoints = 2,
		.isBuilding = false,
		.isUnique = false,
		.cost = Resource{.berry = 2},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			int amount = std::min(3, ge.getPlayer().resources.wood);
			ge.awardResources(Resource{.wood = -amount, .winPoints = amount });
		},
		.imagineProcEffect = [](const GameEngine& ge) {
			int amount = std::min(3, ge.getPlayer().resources.wood);
			return Resource{.wood = -3, .winPoints = 3};
		},
		.idx = (int)ret.size(),
	});

	ret.push_back(Card{
		.name = "Склад",
		.mainBuildingName = "",
		.attachableToName = "",
		.color = Color::Green,
		.greenProcOrder = 2,
		.deployable = true,
		.winPoints = 2,
		.isBuilding = true,
		.isUnique = false,
		.cost = Resource{.wood = 1, .smola = 1, .stone = 1},
		.amountInGame = 3,
		.effect = [](GameEngine& ge) {
			const auto res = ge.getBot()->chooseResourcesToGain(
			{
				Resource{.wood = 3},
				Resource{.smola = 2},
				Resource{.berry = 2},
				Resource{.stone = 1},
			});
			ge.getCOI()->resources += res;
		},
		.deployEffect = [](GameEngine& ge) {
			ge.awardResources(ge.getCOI()->resources);
			ge.getCOI()->resources = Resource{};
		},
		.imagineProcEffect = [](const GameEngine& ge) { return Resource{.anyResource = 2}; },
		.imagineDeployEffect = [](const GameEngine& ge) { return ge.getPlayer().getCardInCity(Card::skladCard)->resources; },
		.idx = (int)ret.size(),
	});

	for (const auto& c: ret) {
		cardByName.emplace(c.name, &c);
	}

	for (auto& c : ret) {
		if (!c.attachableToName.empty()) {
			c.attachableToCard = cardByName.at(c.attachableToName);
		}
		if (!c.mainBuildingName.empty()) {
			c.mainBuildingCard = cardByName.at(c.mainBuildingName);
			const_cast<Card*>(c.mainBuildingCard)->residentCard = &c;
		}
	}

	const_cast<Card*>(cardByName["Ферма"])->synergyDuo.push_back(cardByName["Веточник"]);
	const_cast<Card*>(cardByName["Ферма"])->synergyDuo.push_back(cardByName["Лавка"]);
	const_cast<Card*>(cardByName["Ферма"])->synergyDuo.push_back(cardByName["Муж"]);
	const_cast<Card*>(cardByName["Веточник"])->synergyBase = cardByName["Ферма"];
	const_cast<Card*>(cardByName["Лавка"])->synergyBase = cardByName["Ферма"];
	const_cast<Card*>(cardByName["Муж"])->synergyBase = cardByName["Ферма"];
	
	husbandCard = cardByName.at("Муж");
	wifeCard = cardByName.at("Жена");
	traktirCard = cardByName.at("Трактир");
	bardCard = cardByName.at("Бард");
	hightowerCard = cardByName.at("Вышка");
	zemlekopCard = cardByName.at("Землекоп");
	courtCard = cardByName.at("Суд");
	judgeCard = cardByName.at("Судья");
	healerCard = cardByName.at("Лекарь");
	pigeonCard = cardByName.at("Почтальон");
	rangerCard = cardByName.at("Следопыт");
	dvornikCard = cardByName.at("Дворник");
	towerCard = cardByName.at("Башня");
	historicCard = cardByName.at("Историк");
	razvaliniCard = cardByName.at("Развалины");
	lavochnikCard = cardByName.at("Лавочник");
	postCard = cardByName.at("Почта");
	graveyardCard = cardByName.at("Кладбище");
	shepherdCard = cardByName.at("Пастырь");
	monkCard = cardByName.at("Монах");
	dungeonCard = cardByName.at("Темница");
	rezchikCard = cardByName.at("Резчик");
	chapelCard = cardByName.at("Часовня");
	grobovshikCard = cardByName.at("Гробовщик");
	vetochnikCard = cardByName.at("Веточник");
	teacherCard = cardByName.at("Учитель");
	univerCard = cardByName.at("Университет");
	queenCard = cardByName.at("Королева");
	craneCard = cardByName.at("Кран");
	// inventorCard = cardByName.at("Изобретатель");
	traktirshikCard = cardByName.at("Трактирщик");
	skladCard = cardByName.at("Склад");
	farmCard = cardByName.at("Ферма");
	jokerCard = cardByName.at("Шут");

	allTriggers[TriggerType::Tower] = [](GameEngine& ge) {
		Resource bestProfit;
		double bestScore = 0;
		const auto bot = ge.getBot();
		auto& ps = ge.getPlayer();

		assert(ge.getPlayer().hasCardInCity(Card::towerCard));

		auto tower = const_cast<CardInCity*>(ps.getCardInCity(Card::towerCard));
		if (tower == nullptr) {
			return;
		}
		else {
			tower->resources.winPoints--;
		}

		for (const auto& [fa, idx] : enumerate(ge.getGs().forestActions)) {
			if (std::count(fa.deployedWorkers.begin(), fa.deployedWorkers.end(), ge.getActivePlayer())) {
				if (ps.resources >= fa.price) {
					double pts = bot->measureResourcesToGain(fa.profit - fa.price);
					if (pts > bestScore)
					{
						bestScore = pts;
						bestProfit = fa.profit - fa.price;
					}
				}
			}
		}
		for (const auto& [fa, idx] : enumerate(ge.getGs().baseActions)) {
			if (std::count(fa.deployedWorkers.begin(), fa.deployedWorkers.end(), ge.getActivePlayer())) {
				if (ps.resources >= fa.price) {
					double pts = bot->measureResourcesToGain(fa.profit - fa.price);
					if (pts > bestScore)
					{
						bestScore = pts;
						bestProfit = fa.profit - fa.price;
					}
				}
			}
		}

		ge.log("trigger Башня", towerCard);
		ge.awardResources(bestProfit);
	};

	allTriggers[TriggerType::Historic] = [](GameEngine& ge) {
		assert(ge.getPlayer().hasCardInCity(Card::historicCard));
		ge.log("trigger Историк");
		ge.drawFromDeck(1);
	};

	allTriggers[TriggerType::Court] = [](GameEngine& ge) {
		// assert(ge.getPlayer().hasCardInCity(Card::courtCard));
		ge.log("trigger Суд");
		const auto& res = ge.getPlayer().resources;
		ge.awardResources(ge.getBot()->chooseResourcesToGain({
			{.wood = 1 },
			{.smola = 1 },
			{.stone = 1 },
		}));
	};

	allTriggers[TriggerType::Lavochnik] = [](GameEngine& ge)
	{
		assert(ge.getPlayer().hasCardInCity(Card::lavochnikCard));
		ge.log("trigger Лавочник");
		ge.awardResources({ .berry = 1 });
	};

	return ret;
}

std::vector<Card> Card::allCardsInGame = Card::genAllCards();
