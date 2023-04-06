#pragma once

#include <string>
#include <numeric>

struct ResourceWeights;

struct Resource
{
	int wood = 0;
	int smola = 0;
	int stone = 0;
	int berry = 0;
	int card = 0;
	int anyResource = 0;
	int winPoints = 0;

	double cmc() const;

	std::string to_string() const;

	void removeMostValueable(const Resource& pr);

	Resource operator+(const Resource& r) const {
		return Resource{
			wood + r.wood,
			smola + r.smola,
			stone + r.stone,
			berry + r.berry,
			card + r.card,
			anyResource + r.anyResource,
			winPoints + r.winPoints,
		};
	}

	Resource operator*(const int r) const {
		return Resource{
			wood * r,
			smola * r,
			stone * r,
			berry * r,
			card * r,
			anyResource * r,
			winPoints * r,
		};
	}

	ResourceWeights operator*(double r) const;

	void operator+=(const Resource& r) {
		wood += r.wood;
		smola += r.smola;
		stone += r.stone;
		berry += r.berry;
		card += r.card;
		anyResource += r.anyResource;
		winPoints += r.winPoints;
	}

	void operator-=(const Resource& r) {
		wood -= r.wood;
		smola -= r.smola;
		stone -= r.stone;
		berry -= r.berry;
		card -= r.card;
		anyResource -= r.anyResource;
		winPoints -= r.winPoints;
	}

	Resource operator-(const Resource& r) const {
		Resource ret(*this);
		ret -= r;
		return ret;
	}

	bool operator>=(const Resource& r) const {
		int any = anyResource;
		const int spareResources = wood + smola + stone + berry - r.wood - r.smola - r.stone - r.berry;
		any -= std::max(r.anyResource - spareResources, 0);
		if (any < 0) {
			return false;
		}
		if (any == 0) {
			return
				wood >= r.wood &&
				smola >= r.smola &&
				stone >= r.stone &&
				berry >= r.berry &&
				card >= r.card;
		} else {
			any -= std::max(r.wood - wood, 0) + std::max(r.smola - smola, 0) + std::max(r.berry - berry, 0) + std::max(r.stone - stone, 0);
			if (any < 0) {
				return false;
			} else {
				return true;
			}
		}
	}

	// bool operator<=(const Resource& r) const {
	// 	int spareResources = wood + smola + stone + berry - r.wood - r.smola - r.stone - r.berry;
	// 	return
	// 		wood <= r.wood &&
	// 		smola <= r.smola &&
	// 		stone <= r.stone &&
	// 		berry <= r.berry &&
	// 		card <= r.card &&
	// 		spareResources <= r.anyResource;
	// }
};

struct ResourceWeights
{
	double wood = 0;
	double smola = 0;
	double stone = 0;
	double berry = 0;
	double card = 0;
	// double anyResource = 0;
	double winPoints = 0;

	double operator*(const Resource& r) const;

	void operator+=(const ResourceWeights& r);

	void operator-=(const ResourceWeights& r);

	std::string to_string() const;
};
