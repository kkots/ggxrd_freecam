#include "pch.h"
#include "Game.h"
#include "memoryFunctions.h"
#include "Detouring.h"
#include "Settings.h"
#include "logging.h"

const char** aswEngine = nullptr;

Game game;

bool Game::onDllMain() {
	bool error = false;

	char aswEngineSig[] = "\x85\xC0\x78\x74\x83\xF8\x01";
	aswEngine = (const char**)sigscanOffset(
		"GuiltyGearXrd.exe",
		aswEngineSig,
		_countof(aswEngineSig),
		{-4, 0},
		&error, "aswEngine");

	char gameDataPtrSig[] = "\x33\xC0\x38\x41\x44\x0F\x95\xC0\xC3\xCC";
	gameDataPtr = (const char**)sigscanOffset(
		"GuiltyGearXrd.exe",
		gameDataPtrSig,
		_countof(gameDataPtrSig),
		{-4, 0},
		NULL, "gameDataPtr");

	if (!error && sigscanFrameByFraming()) {
		hookFrameByFraming();
	}

	orig_determineHitType = (determineHitType_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x55\x8b\xec\x83\xe4\xf8\x81\xec\x54\x02\x00\x00\xa1\x00\x00\x00\x00\x33\xc4\x89\x84\x24\x50\x02\x00\x00\x8b\x45\x10\x53\x56\x8b\x75\x08\x8b\x5e\x10\xf7\xdb\x57\x8b\xf9\x1b\xdb\x23\xde\xf6\x87\x64\x04\x00\x00\x02",
		"xxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		nullptr, "determineHitType");

	if (orig_determineHitType) {
		int(HookHelp::*determineHitTypeHookPtr)(void*, BOOL, unsigned int*, unsigned int*) = &HookHelp::determineHitTypeHook;
		detouring.attach(&(PVOID&)(orig_determineHitType),
			(PVOID&)determineHitTypeHookPtr,
			&orig_determineHitTypeMutex,
			"determineHitType");
	}
	
	postEffectOnPtr = (BOOL*)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x89\x8e\xa4\x01\x00\x00\x8b\x15\x00\x00\x00\x00\x89\x96\xa8\x01\x00\x00\x39\x1d\x00\x00\x00\x00\x0f\x94\xc0\x33\xc9\x89\x86\xac\x01\x00\x00",
		"xxxxxxxx????xxxxxxxx????xxxxxxxxxxx",
		{ 20, 0 },
		nullptr, "postEffectOn");

	return !error;
}

GameMode Game::getGameMode() const {
	if (!gameDataPtr || !(*gameDataPtr)) return GAME_MODE_TRAINING;
	return (GameMode)*(*gameDataPtr + 0x45);
}

bool Game::isGameModeOnline() const {
	GameMode gameMode = getGameMode();
	return !(gameMode == GAME_MODE_ARCADE
		|| gameMode == GAME_MODE_CHALLENGE
		|| gameMode == GAME_MODE_REPLAY
		|| gameMode == GAME_MODE_STORY
		|| gameMode == GAME_MODE_TRAINING
		|| gameMode == GAME_MODE_TUTORIAL
		|| gameMode == GAME_MODE_VERSUS);
}

bool Game::sigscanFrameByFraming() {
	bool error = false;

	orig_gameLoop = (gameLoop_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x83\xec\x18\x53\x55\x57\x8b\x7c\x24\x28\x8b\x87\x48\x01\x00\x00\x33\xdb\x89\x9f\x44\x01\x00\x00\x3b\xc3\x7d\x22\x8b\x87\x40\x01\x00\x00\x89\x9f\x48\x01\x00\x00\x3b\xc3\x74\x12\x6a\x08",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		&error, "gameLoop");

	uintptr_t trainingHudCallPlace = sigscanOffset(
		"GuiltyGearXrd.exe",
		"\xb9\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x84\xc0\x75\x0c\xe8\x00\x00\x00\x00\x8b\xc8\xe8\x00\x00\x00\x00\x8b\x4c\x24\x1c",
		"x????x????xxxxx????xxx????xxxx",
		&error, "trainingHudCallPlace");
	if (trainingHudCallPlace) {
		getTrainingHudArgument = (getTrainingHudArgument_t)followRelativeCall(trainingHudCallPlace + 14);
		trainingHudTick = (trainingHudTick_t)followRelativeCall(trainingHudCallPlace + 21);
	}

	char actualGameLoopSig[] = "\x89\x7c\x24\x14\xe8\x00\x00\x00\x00\x85\xc0\x74\x0a\x6a\x01\xe8\x00\x00\x00\x00\x83\xc4\x04\x8b\x0d\x00\x00\x00\x00\x8b\x81\x00\x00\x00\x00\x8b\x80\x7c\x03\x00\x00";
	char actualGameLoopSigMask[] = "xxxxx????xxxxxxx????xxxxx????xx????xxxxxx";
	substituteWildcard(actualGameLoopSigMask, actualGameLoopSig, (char*)&aswEngine, 4, 2);
	orig_actualGameLoop = (actualGameLoop_t)((sigscanOffset(
		"GuiltyGearXrd.exe",
		actualGameLoopSig,
		actualGameLoopSigMask,
		&error, "actualGameLoop") - 0x30) & 0xFFFFFFF0);
	logwrap(fprintf(logfile, "Final location of actualGameLoop: %p\n", orig_actualGameLoop));

	orig_updateAnimations = (updateAnimations_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x57\xbf\x01\x00\x00\x00\x39\x7c\x24\x20\x75\x18\x8b\x06\x8b\x90\xec\x01\x00\x00\x8b\xce\xff\xd2\xc7\x44\x24\x10\x00\x00\x00\x00",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		{ -8 },
		&error, "updateAnimations");

	return !error;
}

void Game::hookFrameByFraming() {
	
	detouring.attach(&(PVOID&)(orig_gameLoop),
		Game::gameLoopHookStatic,
		&orig_gameLoopMutex,
		"gameLoop");

	void(HookHelp::*actualGameLoopHookPtr)(int param1) = &HookHelp::actualGameLoopHook;
	detouring.attach(&(PVOID&)orig_actualGameLoop,
		(PVOID&)actualGameLoopHookPtr,
		&orig_actualGameLoopMutex,
		"actualGameLoop");

	detouring.attach(&(PVOID&)(orig_updateAnimations),
		Game::updateAnimationsHookStatic,
		&orig_updateAnimationsMutex,
		"updateAnimations");

}

void Game::updateAnimationsHookStatic(int param1, int param2, int param3, int param4) {
	HookTracker hookTracker;
	game.updateAnimationsHook(param1, param2, param3, param4);
}

void Game::updateAnimationsHook(int param1, int param2, int param3, int param4) {
	if (ignoreAllCalls) return;
	{
		std::unique_lock<std::mutex> guard(orig_updateAnimationsMutex);
		orig_updateAnimations(param1, param2, param3, param4);
	}
}

void Game::HookHelp::actualGameLoopHook(int param1) {
	HookTracker hookTracker;
	return game.actualGameLoopHook((char*)this, param1);
}

void Game::actualGameLoopHook(char* thisArg, int param1) {
	if (ignoreAllCalls) return;
	{
		std::unique_lock<std::mutex> guard(orig_actualGameLoopMutex);
		orig_actualGameLoop(thisArg, param1);
	}
}

void Game::gameLoopHookStatic(int param1, int param2, int param3, int param4) {
	HookTracker hookTracker;
	game.gameLoopHook(param1, param2, param3, param4);
}

void Game::gameLoopHook(int param1, int param2, int param3, int param4) {
	ignoreAllCalls = false;
	if (freezeGame) {
		slowmoSkipCounter = 0;
		if (!allowNextFrame) {
			ignoreAllCalls = true;
		}
		allowNextFrame = false;
	}
	if (slowmoGame) {
		++slowmoSkipCounter;
		if ((int)slowmoSkipCounter < settings.slowmoTimes) {
			ignoreAllCalls = true;
		}
		else {
			slowmoSkipCounter = 0;
		}
	}
	else {
		slowmoSkipCounter = 0;
	}
	{
		std::unique_lock<std::mutex> guard(orig_gameLoopMutex);
		orig_gameLoop(param1, param2, param3, param4);
	}
	if (ignoreAllCalls) {
		gameLoopHookEmpty();
	}
}

void Game::gameLoopHookEmpty() {
	if (getTrainingHudArgument) {
		trainingHudTick(getTrainingHudArgument());
	}
}

/* 0 means no hit
   1 is get hit (including by all throws)
   2 is blocked hit
   3 is ignoring hit due to playing a possibly very long throw animation (dunno why that is separate)
   4 is rejected hit */
int Game::HookHelp::determineHitTypeHook(void* defender, BOOL wasItType10Hitbox, unsigned int* param3, unsigned int* hpPtr) {
	HookTracker hookTracker;
	int result;
	{
		std::unique_lock<std::mutex> guard(game.orig_determineHitTypeMutex);
		result = game.orig_determineHitType(this, defender, wasItType10Hitbox, param3, hpPtr);
	}
	if (!game.everyoneInvulnerable) return result;
	return 0;
}

BOOL& Game::postEffectOn() {
	static BOOL placeholder = 0;
	if (!postEffectOnPtr) return placeholder;
	return *postEffectOnPtr;
}
