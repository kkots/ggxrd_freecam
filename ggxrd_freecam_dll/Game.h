#pragma once
#include "gameModes.h"

extern const char** aswEngine;

class Game {
public:
	bool onDllMain();
	GameMode getGameMode() const;
	bool isGameModeOnline() const;
	char** gamePtr = nullptr;
private:
	const char** gameDataPtr = nullptr;
};

extern Game game;
