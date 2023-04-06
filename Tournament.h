#pragma once
#include "SimpleBot.h"

struct GameResult
{
	int winner;
	std::vector<int> winPoints;
};

class Tournament
{
public:
	Tournament(std::vector<BotParams> botsParams);

	std::vector<double> playAllInAll(int repeat = 1);

	std::vector<double> playAllVsOne(BotParams hero, int repeat = 1);

	static GameResult playSingleGame(const std::vector<const BotParams*>& botParams, int seed, bool withLogs = false);

private:
	std::vector<BotParams> botsParams_;
};
