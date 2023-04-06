#pragma once

#include <functional>

#include "Resource.h"

struct GameState;
class GameEngine;
class SimpleBot;

using CardEffect = std::function<void(GameEngine&)>;
using ImaginaryCardEffect = std::function<Resource(const GameEngine&)>;
using ImaginaryLaterEffect = std::function<Resource(const GameEngine&, int season)>;
/*
Effects:
...			choose opp to gain berry/card

Bard		choose N, discard
Post		choose N, discard, draw

Tower		choose action from list
Hightower	choose action from list

Traktir		build from list with discount
Court		gain non-berry
Barachol	exchange 2 resources
Sledopit	Move 1 worker
Sklad		choose resources to gain (later)

Dvornik		choose Green in city
Zemlekop	choose Green in enemy city

Univer		choose incity to sacrifice
Razvalini	choose building to sac

Pigeon		choose free card->city from list
Queen		choose free card->city from list
Teacher		choose a card->hand from list
Grobov		discard from list (meadow), choose a card->hand from list (meadow)

---
Carnival	choose resource-set to gain from list
Glasha		discard 2
Tovarniy w.	Choose 2 resources to gain from a couple

*/
