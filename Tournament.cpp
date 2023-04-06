#include "Tournament.h"

#include "GameEngine.h"

Tournament::Tournament(std::vector<BotParams> botsParams)
	: botsParams_(std::move(botsParams))
{
}

std::vector<double> Tournament::playAllInAll(int repeat) {
	std::vector<double> wins(botsParams_.size(), 0.0);
	std::vector<double> games(botsParams_.size(), 0.0);
	
	for (int p0 = 0; p0 < botsParams_.size(); p0++) {
		for (int seed = 0; seed < repeat; seed++) {
			for (int p1 = p0 + 1; p1 < botsParams_.size(); p1++) {
				games[p0] += 2;
				games[p1] += 2;

				if (playSingleGame({ &botsParams_[p0], &botsParams_[p1]}, seed).winner == 0) {
					wins[p0]++;
				} else {
					wins[p1]++;
				}
				if (playSingleGame({ &botsParams_[p1], &botsParams_[p0]}, seed).winner == 0) {
					wins[p1]++;
				} else {
					wins[p0]++;
				}
			}
		}
	}

	std::vector<double> winrates;
	for (int i = 0; i < wins.size(); i++) {
		winrates.push_back(wins[i] / games[i]);
	}
	return winrates;
}

std::vector<double> Tournament::playAllVsOne(BotParams hero, int repeat) {
	std::vector<double> winrates;
	for (int p0 = 0; p0 < botsParams_.size(); p0++) {
		double wins = 0;
		for (int seed = 0; seed < repeat; seed++) {
			if (playSingleGame({ &botsParams_[p0], &hero}, seed).winner == 0) wins++;
			if (playSingleGame({ &hero, &botsParams_[p0]}, seed).winner == 1) wins++;
		}

		winrates.push_back(wins / (repeat * 2));
	}

	return winrates;
}

GameResult Tournament::playSingleGame(const std::vector<const BotParams*>& botParams, int seed, bool withLogs)
{
	GameEngine ge;

	std::vector<SimpleBot> bots;
	for (const auto b: botParams) {
		bots.emplace_back(*b, ge);
	}

	std::vector<SimpleBot*> botsPtrs;
	for (auto& b: bots)	{
		botsPtrs.emplace_back(&b);
	}

	ge.setBots(botsPtrs);
	ge.playGame(seed);

	std::vector<int> winPoints;
	for (const auto& ps: ge.getGs().players) {
		winPoints.push_back(ps.resources.winPoints);
	}

	if (withLogs) {
		ge.renderLogToFile("log_" + std::to_string(seed) + ".html");
	}

	return GameResult{
		(int) std::distance(winPoints.begin(), std::max_element(winPoints.begin(), winPoints.end())),
		winPoints,
	};
}
