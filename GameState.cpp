#include "GameState.h"

#include <cassert>
#include <iomanip>
#include <sstream>

#include "GameEngine.h"
#include "Utils.h"

#include <random>
#include <string>

double Resource::cmc() const {
	return 1.0 * wood + 1.25 * smola + 1.5 * stone + 1.25 * berry + 0.75 * card + 1.7 * anyResource;
}

std::string Resource::to_string() const
{
	std::string ret;
	if (wood != 0) {
		ret += std::to_string(wood) + " wood, ";
	}
	if (smola != 0) {
		ret += std::to_string(smola) + " smola, ";
	}
	if (stone != 0) {
		ret += std::to_string(stone) + " stone, ";
	}
	if (berry != 0) {
		ret += std::to_string(berry) + " berry, ";
	}
	if (card != 0) {
		ret += std::to_string(card) + " card, ";
	}
	if (anyResource != 0) {
		ret += std::to_string(anyResource) + " anyResource, ";
	}
	if (winPoints != 0) {
		ret += std::to_string(winPoints) + " wp, ";
	}

	if (ret.empty()) {
		ret = "<>";
	}

	return ret;
}

void Resource::removeMostValueable(const Resource& pr)
{
	auto demand = pr - *this;
	if (stone > pr.stone) {
		stone--;
	} else if (smola > pr.smola) {
		smola--;
	} else if (berry > pr.berry) {
		berry--;
	} else if (wood > pr.wood) {
		wood--;
	} else if (stone > 0) {
		stone--;
	} else if (smola > 0) {
		smola--;
	} else if (berry > 0) {
		berry--;
	} else if (wood > 0) {
		wood--;
	}
}

ResourceWeights Resource::operator*(double r) const
{
	return ResourceWeights{
		wood * r,
		smola * r,
		stone * r,
		berry * r,
		card * r,
		winPoints * r,
	};
}

double ResourceWeights::operator*(const Resource& r) const
{
	double addition = 0.0;
	if (r.anyResource > 0)	{
		addition += stone * r.anyResource;
	} else if (r.anyResource < 0) {
		addition += wood * r.anyResource;
	}
	return wood * r.wood +
		smola * r.smola +
		stone * r.stone +
		berry * r.berry +
		card * r.card +
		// anyResource * r.anyResource +
		winPoints * r.winPoints +
		addition;
}

void ResourceWeights::operator+=(const ResourceWeights& r)
{
	wood += r.wood;
	smola += r.smola;
	stone += r.stone;
	berry += r.berry;
	card += r.card;
	winPoints += r.winPoints;
}

void ResourceWeights::operator-=(const ResourceWeights& r)
{
	wood -= r.wood;
	smola -= r.smola;
	stone -= r.stone;
	berry -= r.berry;
	card -= r.card;
	winPoints -= r.winPoints;
}

std::string ResourceWeights::to_string() const
{
	std::stringstream ss;
	ss << std::setprecision(3);
	if (wood != 0) {
		ss << wood << " wood, ";
	}
	if (smola != 0) {
		ss << smola << " smola, ";
	}
	if (stone != 0) {
		ss << stone << " stone, ";
	}
	if (berry != 0) {
		ss << berry << " berry, ";
	}
	if (card != 0) {
		ss << card << " card, ";
	}
		
	if (winPoints != 0) {
		ss << winPoints << " wp, ";
	}

	std::string ret = ss.str();
	if (ret.empty()) {
		ret = "<>";
	}

	return ret;
}

std::vector<FieldAction> genBaseActions() {
	return {
		FieldAction{.idx = 29, .profit = Resource{ .wood = 3 } },
		FieldAction{.idx = 28, .profit = Resource{ .wood = 2, .card = 1 }, .slots = 10 },
		FieldAction{.idx = 27, .profit = Resource{ .smola = 2 } },
		FieldAction{.idx = 26, .profit = Resource{ .smola = 1, .card = 1 }, .slots = 10 },
		FieldAction{.idx = 25, .profit = Resource{ .card = 2, .winPoints = 1}, .slots = 10},
		FieldAction{.idx = 24, .profit = Resource{ .stone = 1 } },
		FieldAction{.idx = 23, .profit = Resource{ .berry = 1, .card = 1 } },
		FieldAction{.idx = 22, .profit = Resource{ .berry = 1 }, .slots = 10 },

		FieldAction{.idx = 30, .price = Resource{ .card = 2 }, .profit = Resource{.anyResource = 1}, .slots = 10},
		FieldAction{.idx = 30, .price = Resource{ .card = 4 }, .profit = Resource{.anyResource = 2}, .slots = 10},
		FieldAction{.idx = 30, .price = Resource{ .card = 6 }, .profit = Resource{.anyResource = 3}, .slots = 10},
		FieldAction{.idx = 30, .price = Resource{ .card = 8 }, .profit = Resource{.anyResource = 4}, .slots = 10},

		FieldAction{.idx = 31, .price = Resource{ .card = 5 }, .profit = Resource{.winPoints = 5}, .slots = 1},
		FieldAction{.idx = 32, .price = Resource{ .card = 4 }, .profit = Resource{.winPoints = 4}, .slots = 1},
		FieldAction{.idx = 33, .price = Resource{ .card = 3 }, .profit = Resource{.winPoints = 3}, .slots = 1},
		FieldAction{.idx = 34, .price = Resource{ .card = 2 }, .profit = Resource{.winPoints = 2}, .slots = 10},
	};
}

std::vector<FieldAction> genForestActions(std::default_random_engine& rng)
{
	std::vector<FieldAction> allForestActions = {
		// copy base
		// copy forest
		// -3 wood => 3 any
		FieldAction{.idx = 1, .profit = Resource{ .berry = 3 } },
		FieldAction{.idx = 2, .profit = Resource{ .card = 2, .anyResource = 1 } },
		// draw 2x discarded
		FieldAction{.idx = 4, .price = Resource{.card = 3 }, .profit = Resource{ .anyResource = 3 } },
		FieldAction{.idx = 6, .profit = Resource{.stone = 1 , .card = 3} },
		FieldAction{.idx = 7, .profit = Resource{.berry = 2, .card = 1 } },
		FieldAction{.idx = 8, .profit = Resource{.wood = 1 , .smola = 2} },
		FieldAction{.idx = 9, .profit = Resource{.anyResource = 2 } },
		FieldAction{.idx = 10, .profit = Resource{.wood = 1, .smola = 1, .berry = 1 } },
		FieldAction{.idx = 12, .profit = Resource{.stone = 2, .card = 1 } },
		// 2 smola / 2 berry
		// smola + stone / 4 card
		// proc 2 green
		
		FieldAction{.idx = 14, .profit = Resource{.stone = 1, .berry = 1, .card = 1 } },
		FieldAction{.idx = 17, .profit = Resource{.smola = 3 } },
		FieldAction{.idx = 19, .profit = Resource{.wood = 4 } },
		FieldAction{.idx = 20, .profit = Resource{.wood = 2, .smola = 1 } },
	};

	rshuffle(allForestActions, rng);
	allForestActions.resize(3);
	return allForestActions;
}

std::vector<Quest> genAllQuests(std::default_random_engine& rng)
{
	std::vector<Quest> nonbasicQuests = {
		// +3 wp per monastery worker
		// +3 per husband+wife in every city
		// return worker, +4 wp
		// look at 5 cards, put some in hand and +1 wp for other
		// pay 3 anyRes to opp, gain +6wp
		// +3 wp for each in GY
		// +3 wp for each in dungeon
		// +2 wp for each in chapel
		// pay 3, gain 1 for wood/berry and 2 for smola/stone

		Quest{
			.idx = 2,
			.imaginaryPrice = Resource{ .berry = 3 },
			.cardsLeft = [=](const PlayerState& ps) { return 2 - ps.hasCardInCity(Card::traktirCard) - ps.hasCardInCity(Card::bardCard); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->name == "Трактир" || card->name == "Бард"; },
			.effect = [](GameEngine& ge) { ge.awardResources(Resource{ .berry = -3, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 3,
			.imaginaryPrice = Resource{ .wood = 3 },
			.cardsLeft = [](const PlayerState& ps) { return 2 - ps.hasCardInCity(Card::hightowerCard) - ps.hasCardInCity(Card::zemlekopCard); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->name == "Вышка" || card->name == "Землекоп"; },
			.effect = [](GameEngine& ge) { ge.awardResources(Resource{.wood = -3, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 4,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) {
				const auto& colors = ps.coloredCardsLookup;
				int ret = 0;
				for (int i = 0; i < 5; i++) {
					ret += 2 - std::min(2, colors[i]);
				}
				return ret;
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				return ps.coloredCardsLookup[card->color] < 2;
			},
			.effect = [](GameEngine& ge) { ge.awardWp(9); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 9 }; }
		},
		Quest{
			.idx = 6,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps)
			{
				// const auto values = ge.getBot()->measureCardsInCityToSac(false);
				int niceToSac = std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return !c.cardInfo->isBuilding; });
				// double th = ge.getBot()->measureResourcesToGain(Resource{.winPoints = 3 });
				// const auto& city = ge.getPlayer().city;
				// for (int i = 0; i < values.size(); i++) {
				//	if (values[i] < th && (!city[i].cardInfo->isBuilding)) {
				//		niceToSac++;
				//	}
				// }

				niceToSac = std::min(2, niceToSac);

				return 4 - niceToSac - ps.hasCardInCity(Card::courtCard) - ps.hasCardInCity(Card::rangerCard);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->name == "Суд" || card->name == "Следопыт"; },
			.effect = [](GameEngine& ge) {
				ge.chooseAndSacFromCity(2, true);
				ge.awardWp(6);
			},
			.imaginaryEffect = [](const GameEngine& ge)
			{
				return Resource{ .winPoints = 6 };
			}
		},
		Quest{
			.idx = 13,
			.imaginaryPrice = Resource{ .berry = 2 },
			.cardsLeft = [](const PlayerState& ps) { return 2 - ps.hasCardInCity(Card::grobovshikCard) - ps.hasCardInCity(Card::vetochnikCard); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->idx == Card::grobovshikCard->idx || card->idx == Card::vetochnikCard->idx; },
			.effect = [](GameEngine& ge) {
				ge.chooseAndSacFromCity(2, false);
				ge.awardResources({ .berry = -2, .winPoints = 6 });
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 15,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) { return 2 - ps.hasCardInCity(Card::teacherCard) - ps.hasCardInCity(Card::univerCard); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->idx == Card::teacherCard->idx || card->idx == Card::univerCard->idx; },
			.effect = [](GameEngine& ge) {
				ge.chooseAndDiscard(3);
				ge.awardWp(6);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 16,
			.imaginaryPrice = Resource{ },
			.cardsLeft = [](const PlayerState& ps)
			{
				return 2 - ps.hasCardInCity(Card::judgeCard) - ps.hasCardInCity(Card::queenCard);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->idx == Card::judgeCard->idx || card->idx == Card::queenCard->idx; },
			.effect = [](GameEngine& ge) {
				for (const auto& c : ge.getPlayer().city) {
					if (c.cardInfo->color == Color::Green) {
						ge.procEffect(const_cast<CardInCity*>(&c));
					}
				}
				ge.awardWp(3);
			},
			.imaginaryEffect = [](const GameEngine& ge) {
				Resource ret;
				for (const auto& c : ge.getPlayer().city) {
					if (c.cardInfo->color == Color::Green) {
						ret += c.cardInfo->imagineProcEffect(ge);
					}
				}
				ret.winPoints += 3;
				return ret;
			}
		},
		Quest{
			.idx = 17,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) {
				return 5 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return !c.cardInfo->isBuilding && !c.cardInfo->isUnique; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return !card->isBuilding && !card->isUnique; },
			.effect = [](GameEngine& ge) { ge.awardWp(5); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 18,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) {
				return 4 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return !c.cardInfo->isBuilding && c.cardInfo->isUnique; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return !card->isBuilding && card->isUnique; },
			.effect = [](GameEngine& ge) { ge.awardWp(5); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 19,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) {
				return 5 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return c.cardInfo->isBuilding && !c.cardInfo->isUnique; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->isBuilding && !card->isUnique; },
			.effect = [](GameEngine& ge) { ge.awardWp(5); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 20,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) {
				return 4 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return c.cardInfo->isBuilding && c.cardInfo->isUnique; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->isBuilding && card->isUnique; },
			.effect = [](GameEngine& ge) { ge.awardWp(5); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 21,
			.imaginaryPrice = Resource{ .stone = 1 },
			.cardsLeft = [](const PlayerState& ps) { return 4 - ps.coloredCardsLookup[Color::Blue]; },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->color == Color::Blue; },
			.effect = [](GameEngine& ge) { ge.awardResources({ .stone = -1, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 22,
			.imaginaryPrice = Resource{ .berry = 2 },
			.cardsLeft = [](const PlayerState& ps) { return 5 - ps.coloredCardsLookup[Color::Green]; },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->color == Color::Green; },
			.effect = [](GameEngine& ge) { ge.awardResources({ .berry = -2, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 23,
			.imaginaryPrice = Resource{.wood = 2 },
			.cardsLeft = [](const PlayerState& ps) { return 4 - ps.coloredCardsLookup[Color::Grey]; },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->color == Color::Grey; },
			.effect = [](GameEngine& ge) { ge.awardResources({ .wood = -2, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 24,
			.imaginaryPrice = Resource{.smola = 2 },
			.cardsLeft = [](const PlayerState& ps) { return 4 - ps.coloredCardsLookup[Color::Red]; },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->color == Color::Red; },
			.effect = [](GameEngine& ge) { ge.awardResources({ .smola = -2, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 25,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) { return 4 - ps.coloredCardsLookup[Color::Purple]; },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->color == Color::Purple; },
			.effect = [](GameEngine& ge) { ge.awardResources({.card = -3, .winPoints = 6 }); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 6 }; }
		},
		Quest{
			.idx = 26,
			.imaginaryPrice = Resource{},
			.cardsLeft = [](const PlayerState& ps) { return 10 - (int) ps.city.size(); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return ps.city.size() < 10; },
			.effect = [](GameEngine& ge) { 
				for (const auto& q : ge.getPlayer().finishedQuests) {
					if (q.isBasic) {
						ge.awardWp(1);
					} else {
						ge.awardWp(2);
					}
				}
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = (int) ge.getPlayer().finishedQuests.size() * 3 / 2}; }
		},
		Quest{
			.idx = 28,
			.imaginaryPrice = Resource{ },
			.cardsLeft = [](const PlayerState& ps) { 
				return 9 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return c.cardInfo->isBuilding; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return card->isBuilding; },
			.effect = [](GameEngine& ge) { ge.awardResources({ .winPoints = 5}); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 27,
			.imaginaryPrice = Resource{ },
			.cardsLeft = [](const PlayerState& ps) {
				return 9 - std::count_if(ps.city.begin(), ps.city.end(),[&](const auto& c) { return !c.cardInfo->isBuilding; }); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return !card->isBuilding; },
			.effect = [](GameEngine& ge) { ge.awardResources({.winPoints = 5}); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 34,
			.imaginaryPrice = Resource{ },
			.cardsLeft = [](const PlayerState& ps) { return 3 - (int) ps.finishedQuests.size(); },
			.isCardForQuest = [](const PlayerState& ps, const Card* card) { return false; },
			.effect = [](GameEngine& ge) { ge.awardResources({.winPoints = 5}); },
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; }
		},
		Quest{
			.idx = 29,
			.imaginaryPrice = Resource{.card = 4 },
			.cardsLeft = [](const PlayerState& ps) {
				const auto& cnt = ps.coloredCardsLookup;
				return std::max(0, 3 - cnt[Color::Green]) + std::max(0, 2 - cnt[Color::Grey]);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				if (card->color == Color::Green) {
					return ps.coloredCardsLookup[card->color] < 3;
				} else if (card->color == Color::Grey) {
					return ps.coloredCardsLookup[card->color] < 2;
				}
				return false;
			},
			.effect = [](GameEngine& ge) {
				auto& ps = ge.getPlayer();
				auto cnt = ps.coloredCardsLookup[Color::Green];
				if (cnt > ps.hand.size()) {
					cnt = ps.hand.size();
					while (ps.hand.size() > 0) {
						ge.discard(0);
					}
				} else {
					ge.chooseAndDiscard(cnt);
				}
				ge.awardWp(3 + cnt);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 7 }; }
		},
		Quest{
			.idx = 30,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) {
				const auto& cnt = ps.coloredCardsLookup;
				return std::max(0, 2 - cnt[Color::Blue]) + std::max(0, 2 - cnt[Color::Grey]);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				if (card->color == Color::Blue) {
					return ps.coloredCardsLookup[card->color] < 2;
				}
			 else if (card->color == Color::Grey) {
			  return ps.coloredCardsLookup[card->color] < 2;
			}
				return false;
			},
			.effect = [](GameEngine& ge) {
				auto& ps = ge.getPlayer();
				auto cnt = ps.coloredCardsLookup[Color::Grey];
				if (cnt > ps.hand.size()) {
					cnt = ps.hand.size();
					while (ps.hand.size() > 0) {
						ge.discard(0);
					}
				}
				else {
					ge.chooseAndDiscard(cnt);
				}
				ge.awardWp(4 + cnt);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 7 }; }
		},
		Quest{
			.idx = 31,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) {
				const auto& cnt = ps.coloredCardsLookup;
				return std::max(0, 2 - cnt[Color::Red]) + std::max(0, 2 - cnt[Color::Purple]);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				if (card->color == Color::Red) {
					return ps.coloredCardsLookup[card->color] < 2;
				}
			 else if (card->color == Color::Purple) {
			  return ps.coloredCardsLookup[card->color] < 2;
			}
				return false;
			},
			.effect = [](GameEngine& ge) {
				auto& ps = ge.getPlayer();
				auto cnt = ps.coloredCardsLookup[Color::Red];
				if (cnt > ps.hand.size()) {
					cnt = ps.hand.size();
					while (ps.hand.size() > 0) {
						ge.discard(0);
					}
				}
				else {
					ge.chooseAndDiscard(cnt);
				}
				ge.awardWp(4 + cnt);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 7 }; }
		},
		Quest{
			.idx = 32,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) {
				const auto& cnt = ps.coloredCardsLookup;
				return std::max(0, 3 - cnt[Color::Green]) + std::max(0, 2 - cnt[Color::Purple]);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				if (card->color == Color::Green) {
					return ps.coloredCardsLookup[card->color] < 3;
				}
			 else if (card->color == Color::Purple) {
			  return ps.coloredCardsLookup[card->color] < 2;
			}
				return false;
			},
			.effect = [](GameEngine& ge) {
				auto& ps = ge.getPlayer();
				auto cnt = ps.coloredCardsLookup[Color::Purple];
				if (cnt > ps.hand.size()) {
					cnt = ps.hand.size();
					while (ps.hand.size() > 0) {
						ge.discard(0);
					}
				}
				else {
					ge.chooseAndDiscard(cnt);
				}
				ge.awardWp(4 + cnt);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 7 }; }
		},
		Quest{
			.idx = 33,
			.imaginaryPrice = Resource{.card = 3 },
			.cardsLeft = [](const PlayerState& ps) {
				const auto& cnt = ps.coloredCardsLookup;
				return std::max(0, 2 - cnt[Color::Blue]) + std::max(0, 2 - cnt[Color::Red]);
			},
			.isCardForQuest = [](const PlayerState& ps, const Card* card)
			{
				if (card->color == Color::Blue) {
					return ps.coloredCardsLookup[card->color] < 2;
				}
			 else if (card->color == Color::Red) {
			  return ps.coloredCardsLookup[card->color] < 2;
			}
				return false;
			},
			.effect = [](GameEngine& ge) {
				auto& ps = ge.getPlayer();
				auto cnt = ps.coloredCardsLookup[Color::Blue];
				if (cnt > ps.hand.size()) {
					cnt = ps.hand.size();
					while (ps.hand.size() > 0) {
						ge.discard(0);
					}
				}
				else {
					ge.chooseAndDiscard(cnt);
				}
				ge.awardWp(4 + cnt);
			},
			.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 7 }; }
		},
	};
	
	std::vector<Quest> res = nonbasicQuests;
	rshuffle(res, rng);
	res.resize(4);

	// add basic quests
	res.push_back(Quest{
		.idx = 35,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 4 - ps.coloredCardsLookup[Color::Green]; },
		.isCardForQuest = [](const PlayerState& ps, const Card* card)
		{
			return card->color == Color::Green;
		},
		.effect = [](GameEngine& ge) { ge.awardWp(3); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 3 }; },
		.isBasic = true
	});
	res.push_back(Quest{
		.idx = 39,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 3 - ps.coloredCardsLookup[Color::Red]; },
		.isCardForQuest = [](const PlayerState& ps, const Card* card)
		{
			return card->color == Color::Red;
		},
		.effect = [](GameEngine& ge) { ge.awardWp(3); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 3 }; },
		.isBasic = true,
	});
	res.push_back(Quest{
		.idx = 36,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 3 - ps.coloredCardsLookup[Color::Blue]; },
		.isCardForQuest = [](const PlayerState& ps, const Card* card)
		{
			return card->color == Color::Blue;
		},
		.effect = [](GameEngine& ge) { ge.awardWp(3); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 3 }; },
		.isBasic = true,
	});
	res.push_back(Quest{
		.idx = 40,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 3 - ps.coloredCardsLookup[Color::Grey]; },
		.isCardForQuest = [](const PlayerState& ps, const Card* card)
		{
			return card->color == Color::Grey;
		},
		.effect = [](GameEngine& ge) { ge.awardWp(3); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 3 }; },
		.isBasic = true,
	});
	res.push_back(Quest{
		.idx = 37,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 3 - ps.coloredCardsLookup[Color::Purple]; },
		.isCardForQuest = [](const PlayerState& ps, const Card* card)
		{
			return card->color == Color::Purple;
		},
		.effect = [](GameEngine& ge) { ge.awardWp(3); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 3 }; },
		.isBasic = true
	});
	res.push_back(Quest{
		.idx = 41,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) { return 15 - (int) ps.city.size(); },
		.isCardForQuest = [](const PlayerState& ps, const Card* card) { return ps.city.size() < 15; },
		.effect = [](GameEngine& ge) { ge.awardWp(5); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 5 }; },
		.isBasic = true
	});
	res.push_back(Quest{
		.idx = 38,
		.imaginaryPrice = Resource{},
		.cardsLeft = [](const PlayerState& ps) {
			int ret = 0;
			for (const auto& c : ps.coloredCardsLookup) {
				if (c == 0) {
					ret++;
				}
			}
			return ret;
		},
		.isCardForQuest = [](const PlayerState& ps, const Card* card) { return ps.coloredCardsLookup[card->color] == 0; },
		.effect = [](GameEngine& ge) { ge.awardWp(4); },
		.imaginaryEffect = [](const GameEngine& ge) { return Resource{.winPoints = 4 }; },
		.isBasic = true
	});

	return res;
}

int countInCityByName(const PlayerState& ps, const std::string& name)
{
	return std::count_if(ps.city.begin(), ps.city.end(), [&](const auto& c) { return c.cardInfo->name == name; });
}
