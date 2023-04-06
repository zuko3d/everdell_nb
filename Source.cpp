#include <algorithm>
#include <vector>
#include <iostream>
#include <functional>
#include <string>
#include <cstdlib>
#include <random>
#include <sstream>
#include <numeric>
#include <unordered_map>

// #include "omp.h"

// #define NOMINMAX
// #include <Windows.h>
// #define NOMINMAX

#include "GameEngine.h"
#include "Tournament.h"

std::random_device rd;
std::mt19937 g(rd());

void printStats(const std::vector<double>& vec) {
	double mean = 0.0;
	for (const auto d: vec)	{
		mean += d;
	}
	mean /= vec.size();
	double dev = 0.0;
	for (const auto d : vec) {
		dev += (d - mean) * (d - mean);
	}
	dev = sqrt(dev / vec.size());

	std::cout << mean << " +- " << dev << ", TS: " << mean - 3 * dev << std::endl;
}

/*
std::vector<Card> cards = {
	// Purple
	Card{"Architect", "Crane", Purple, 2 + 3, false, true, Resource{.berry = 4}, noPoints, [&](const std::vector<int>&) { return Resource{.smola = -3}; }, "3"},
	Card{"Wife", "", Purple, 5, false, false, Resource{.berry = 2}, noPoints, noResources},
	Card{"King", "Zamok", Purple, 5, false, true, Resource{.berry = 6}, noPoints, noResources},

	Card{"Theatre", "", Purple, 3, true, true, Resource{.wood = 3, .smola = 1,.stone = 1}, [&](const std::vector<int>& city) {
		return std::count_if(city.begin(), city.end(),[&](const int& c) { return !cards[c].isBuilding && cards[c].isUnique; });
	}, noResources},
	Card{"School", "", Purple, 2, true, true, Resource{.wood = 2, .smola = 2}, [&](const std::vector<int>& city) {
		return std::count_if(city.begin(), city.end(),[](const int& c) { return !cards[c].isBuilding && !cards[c].isUnique; });
	}, noResources},
	Card{"Zamok", "", Purple, 4, true, true, Resource{.wood = 2, .smola = 3,.stone = 3}, [&](const std::vector<int>& city) {
		return std::count_if(city.begin(), city.end(),[&](const int& c) { return cards[c].isBuilding && !cards[c].isUnique; });
	}, noResources},
	Card{"Palace", "", Purple, 4, true, true, Resource{.wood = 2, .smola = 3,.stone = 3}, [&](const std::vector<int>& city) {
		return std::count_if(city.begin(), city.end(),[&](const int& c) { return cards[c].isBuilding && cards[c].isUnique; });
	}, noResources},
	Card{"Tree", "", Purple, 5, true, true, Resource{.wood = 3, .smola = 3,.stone = 3}, [&](const std::vector<int>& city) {
		return std::count_if(city.begin(), city.end(),[&](const int& c) { return cards[c].color == Purple; });
	}, noResources},

	// Blue
	Card{"Historic", "Tower", Blue, 1, false, true, Resource{.berry = 2}, noPoints, [&](const std::vector<int>&) { return Resource{.card = 11}; }},
	Card{"Judge", "Court", Blue, 2, false, true, Resource{.berry = 3}, noPoints, [&](const std::vector<int>&) { return Resource{.anyResource = 2}; }},
	Card{"Lavochnik", "Lavka", Blue, 1, false, true, Resource{.berry = 2}, noPoints, [&](const std::vector<int>& city) {
		return Resource{.berry = (int) std::count_if(city.begin(), city.end(),[&](const int& c) { return !cards[c].isBuilding; }) - 3};
	}},
	
	Card{"Court", "", Blue, 2, true, true, Resource{.wood = 1, .smola = 1,.stone = 2}, noPoints, [&](const std::vector<int>& city) {
		return Resource{.stone = (int) std::count_if(city.begin(), city.end(),[&](const int& c) { return cards[c].isBuilding; }) - 3};
	}},

	// Red
	Card{"Queen", "Palace", Red, 4, true, true, Resource{.berry = 5}, noPoints, noResources},

	Card{"Univer", "", Red, 5, true, true, Resource{.smola = 1,.stone = 2}, noPoints, [](const std::vector<int>&) { return Resource{.anyResource = 2}; }},

	// Green 
	Card{"Vetochnik", "Vetkovozka", Green, 2, false, false, Resource{.berry = 2}, noPoints, [&](const std::vector<int>& city) {
		return Resource{.wood = 2 * 2 * (int) std::count_if(city.begin(), city.end(),[&](auto c) { return cards[c].name == "Farm"; })};
	}},
	Card{"Rezchik", "Sklad", Green, 2, false, false, Resource{.berry = 2}, noPoints, [](const std::vector<int>&) { return Resource{.wood = 0 * 2}; }, "0"},
	Card{"Rezchik", "Sklad", Green, 2 + 4, false, false, Resource{.berry = 2}, noPoints, [](const std::vector<int>&) { return Resource{.wood = -2 * 2}; }, "4"},
	Card{"Rezchik", "Sklad", Green, 2 + 6, false, false, Resource{.berry = 2}, noPoints, [](const std::vector<int>&) { return Resource{.wood = -3 * 2}; }, "6"},
	Card{"Dvornik", "Smolo", Green, 2, false, false, Resource{.berry = 3}, noPoints, [](const std::vector<int>&) { return Resource{.wood = 2, .berry = 1, .anyResource = 1}; }},
	Card{"Miner", "Mine", Green, 1, false, false, Resource{.berry = 3}, noPoints, [](const std::vector<int>&) { return Resource{.wood = 2, .berry = 1, .anyResource = 1}; }},
	Card{"Husband", "Farm", Green, 2, false, false, Resource{.berry = 3}, noPoints, [](const std::vector<int>&) { return Resource{.berry = 2}; }},
	Card{"Healer", "Univer", Green, 4 + 3, false, true, Resource{.berry = 4}, noPoints, [](const std::vector<int>&) { return Resource{.berry = -3}; }, "3"},
	Card{"Healer", "Univer", Green, 4 + 6, false, true, Resource{.berry = 4}, noPoints, [](const std::vector<int>&) { return Resource{.berry = -3 * 2}; }, "6"},
	
	Card{"Sklad", "", Green, 2, true, false, Resource{.wood = 1, .smola = 1,.stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.anyResource = 2 * 2 - 2}; }},
	Card{"Vetkovozka", "", Green, 1, true, false, Resource{.wood = 1, .stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.wood = 2 * 2}; }},
	Card{"Lavka", "", Green, 1, true, false, Resource{.smola = 1,.stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.berry = 2 * 2}; }},
	Card{"Smolo", "", Green, 1, true, false, Resource{.smola = 1,.stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.smola = 2}; }},
	Card{"Mine", "", Green, 2, true, false, Resource{.wood=1, .smola = 1,.stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.stone= 2}; }},
	Card{"Farm", "", Green, 1, true, false, Resource{.wood = 2, .smola = 1}, noPoints, [](const std::vector<int>&) { return Resource{.berry = 2}; }},
	Card{"Yarmarka", "", Green, 3, true, true, Resource{.wood = 1, .smola = 2,.stone = 1}, noPoints, [](const std::vector<int>&) { return Resource{.card = 2 * 2}; }},

	// Grey
	Card{"Bard", "Theatre", Grey, 5, false, true, Resource{.berry = 3}, noPoints, [](const std::vector<int>&) { return Resource{.card = -5}; }},
	Card{"Strannik", "High", Grey, 1, false, false, Resource{.berry = 2}, noPoints, [](const std::vector<int>&) { return Resource{.card = 3}; }},
};

std::unordered_map<std::string, int> cardByName;

struct Stats {
	int points = 0;
	Resource totalCost;
	Resource totalResources;
};

std::vector<int> getCount(std::vector<int> deck) {
	std::vector<int> count(cards.size(), 0);
	for (const auto& d : deck) {
		count[d]++;
	}
	return count;
}

Stats countPoints(const std::vector<int>& city) {
	Stats res;

	const auto count = getCount(city);

	for (int id = 0; id < count.size(); id++) {
		if (count[id] > 0) {
			const auto& c = cards[id];

			for (int j = 0; j < count[id]; j++) {
				res.points += c.winPoints;
				res.points += c.additionalPoints(city);
				res.totalResources += c.additionalResources(city);
			}

			int fed = 0;
			if (c.baseBuildingId >= 0) {
				fed = std::min(count[id], count.at(c.baseBuildingId));
			}

			for (int j = 0; j < count[id] - fed; j++) {
				res.totalCost += c.cost;
			}
		}
	}

	return res;
}

std::vector<int> genPool() {
	std::vector<int> ret;
	ret.reserve(200);
	for (int i = 0; i < cards.size(); i++) {
		if (cards[i].name != "Wife") {
			ret.push_back(i);
		}
	}
	for (int i = 0; i < cards.size(); i++) {
		if (cards[i].name != "Wife") {
			// if (!cards[i].isUnique) ret.push_back(i);
		}
	}

	return ret;
}

std::vector<int> genDeck() {
	static const auto pool = genPool();
	static const int husbandId = std::find_if(cards.begin(), cards.end(), [](const Card& c) { return c.name == "Husband"; }) - cards.begin();
	static const int wifeId = std::find_if(cards.begin(), cards.end(), [](const Card& c) { return c.name == "Wife"; }) - cards.begin();
	static const int strannikId = std::find_if(cards.begin(), cards.end(), [](const Card& c) { return c.name == "Strannik"; }) - cards.begin();
	auto curPool = pool;

	std::shuffle(curPool.begin(), curPool.end(), g);
	int finalSize = 15;
	for (int i = 0; i < finalSize; i++) {
		if (curPool[i] == strannikId) {
			finalSize++;
		}
	}
	curPool.resize(finalSize);

	for (int i : curPool) {
		if (i == husbandId) {
			curPool.push_back(wifeId);
		}
	}

	return curPool;
}
*/
int main() {
	// SetConsoleCP(1251);
	// SetConsoleOutputCP(1251);

	GameEngine ge;

	BotParams botParams1{
		.weights_ = {
			ResourceWeights{
				.wood = 100,
				.smola = 125,
				.stone = 150,
				.berry = 126,
				.card = 60,
				.winPoints = 30,
			},
			ResourceWeights{
				.wood = 100,
				.smola = 125,
				.stone = 150,
				.berry = 126,
				.card = 60,
				.winPoints = 50,
			},
			ResourceWeights{
				.wood = 100,
				.smola = 125,
				.stone = 150,
				.berry = 126,
				.card = 50,
				.winPoints = 80,
			},
			ResourceWeights{
				.wood = 100,
				.smola = 125,
				.stone = 150,
				.berry = 126,
				.card = 50,
				.winPoints = 120,
			}
		},
	};

	botParams1.parse("74.08 118.9 150 126 60 30 74.08 118.9 150 126 60 50 74.08 118.9 150 126 50 80 74.08 118.9 150 126 50 129.3 5 1.2 0.5 1.368 0.2783 2.266 2.926 1.4");

	BotParams b1;
	b1.parse("63.76 125 146.3 114 58.52 30 63.76 125 146.3 114 60 50 63.76 125 146.3 114 50 80 63.76 125 146.3 114 50 129.3 4.877 1.17 0.5389 1.12 0.3 2 3.316 1.365");
	BotParams b2;
	b2.parse("72.25 99.81 146.3 111.2 64.67 30 72.25 99.81 146.3 111.2 60 47.56 72.25 99.81 146.3 111.2 50 80 72.25 99.81 146.3 111.2 50 129.3 5 1.2 0.4094 1.148 0.2926 2.05 3 1.435");
	
	Tournament::playSingleGame({ &b1, &b2 }, 11, true);
	return 0;

	std::vector<std::vector<BotParams>> villages;
	for (int i = 0; i < 8; i++) {
		villages.push_back({});
		auto& mutated = villages.back();
		for (int i = 0; i < 200; i++) {
			mutated.push_back(botParams1);
			mutated.back().mutate(20);
		}
	}

#pragma omp parallel for schedule(dynamic)
	for (auto& village: villages) {
		Tournament tournament(village);
		const auto result = tournament.playAllVsOne(botParams1, 30);
		
		std::vector<BotParams> offspring;
		for (const auto& [wr, idx]: enumerate(result)) {
			if (wr > 0.58) {
				// std::cout << wr << " " << idx << std::endl;
				offspring.push_back(village[idx]);
			}
		}
		village = std::move(offspring);
	}

	for (int gen = 0; gen < 6; gen++) {
		std::cout << "generation " << gen << " ======================" << std::endl;
#pragma omp parallel for schedule(dynamic)
		for (auto& village: villages) {
			std::cout << "village size: " << village.size() << std::endl;
			std::vector<BotParams> nextOffspring;
			for (const auto& v: village) {
				for (int i = 0; i < 4; i++) {
					nextOffspring.push_back(v);
					nextOffspring.back().mutate(2);
				}
			}

			Tournament otournament(nextOffspring);
			const auto winrates = otournament.playAllInAll(10000 / nextOffspring.size() / nextOffspring.size());
			auto best = argsort(winrates);
			std::reverse(best.begin(), best.end());
			best.resize(8);
			village.clear();
			std::cout << winrates[best[0]] << " | " << nextOffspring[best[0]].serialize() << std::endl;
			for (const auto idx: best) {
				village.push_back(nextOffspring[idx]);
			}
		}
	}


	std::cout << " *********** ========= *********** " << std::endl;
	std::vector<BotParams> bestOfVillages;
	for (const auto& v: villages) {
		bestOfVillages.push_back(v[0]);
	}

	Tournament otournament(bestOfVillages);
	const auto winrates = otournament.playAllInAll(10000 / bestOfVillages.size() / bestOfVillages.size());
	auto best = argsort(winrates);
	std::reverse(best.begin(), best.end());
	
	for (const auto idx: best) {
		std::cout << winrates[idx] << " | " << bestOfVillages[idx].serialize() << std::endl;
	}
	// double bestSum = 0;
	// double worstSum = 10000;

	// double totalPoints = 0;
	// int games = 0;
	// std::vector<std::vector<double>> wp(2);
	// double wins = 0;
	// double wins_p0 = 0;

	// for (const auto& [r1, idx1]: enumerate(result)) {
	// 	for (const auto& [r2, idx2] : enumerate(r1)) {
	// 		for (const auto& [r3, idx3] : enumerate(r2)) {
	// 			int sum = r3.winPoints[0] + r3.winPoints[1];
	// 			totalPoints += sum;
	// 			if (r3.winPoints[0] > r3.winPoints[1]) {
	// 				wins++;
	// 			}
	// 			if (r3.winPoints[idx1] > r3.winPoints[idx2]) {
	// 				wins_p0++;
	// 			}
	// 			wp[idx1].push_back(r3.winPoints[0]);
	// 			wp[idx2].push_back(r3.winPoints[1]);
	// 			games++;
	// 			if (bestSum < sum) {
	// 				bestSum = sum;
	// 				std::cout << idx1 << " " << idx2 << " " << idx3 << " " << bestSum << std::endl;
	// 			}
	// 			if (worstSum > std::min(r3.winPoints[0], r3.winPoints[1])) {
	// 				worstSum = std::min(r3.winPoints[0], r3.winPoints[1]);
	// 				std::cout << idx1 << " " << idx2 << " " << idx3 << " " << worstSum << std::endl;
	// 			}
	// 		}
	// 	}
	// }

	// std::cout << "winrate 1st player: " << wins / games << std::endl;
	// std::cout << "winrate bot1: " << wins_p0 / games << std::endl;
	// printStats(wp[0]);
	// printStats(wp[1]);
	
	// ge.renderLogToFile("log.html");
	return 0;
}

