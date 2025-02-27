#pragma once
#include "gameModes.h"
#include <mutex>

using determineHitType_t = int(__thiscall*)(void*, void*, BOOL, unsigned int*, unsigned int*);

extern const char** aswEngine;

using gameLoop_t = void(__cdecl*)(int param1, int param2, int param3, int param4);
using actualGameLoop_t = void(__thiscall*)(char* thisArg, int param1);
using trainingHudTick_t = void(__thiscall*)(char* thisArg);
using getTrainingHudArgument_t = char* (__cdecl*)(void);
using updateAnimations_t = void(__cdecl*)(int param1, int param2, int param3, int param4);

class Game {
public:
	bool onDllMain();
	GameMode getGameMode() const;
	bool isGameModeOnline() const;
	char** gamePtr = nullptr;
	bool freezeGame = false;
	bool slowmoGame = false;
	bool allowNextFrame = false;
	determineHitType_t orig_determineHitType = nullptr;
	std::mutex orig_determineHitTypeMutex;
	bool everyoneInvulnerable = false;
	BOOL& postEffectOn();
private:
	class HookHelp {
		friend class Game;
		void actualGameLoopHook(int param1);
		int determineHitTypeHook(void* defender, BOOL wasItType10Hitbox, unsigned int* param3, unsigned int* hpPtr);
	};
	bool sigscanFrameByFraming();
	void hookFrameByFraming();
	void static __cdecl updateAnimationsHookStatic(int param1, int param2, int param3, int param4);
	void updateAnimationsHook(int param1, int param2, int param3, int param4);
	void actualGameLoopHook(char* thisArg, int param1);
	void static __cdecl gameLoopHookStatic(int param1, int param2, int param3, int param4);
	void gameLoopHook(int param1, int param2, int param3, int param4);
	void gameLoopHookEmpty();
	gameLoop_t orig_gameLoop = nullptr;
	std::mutex orig_gameLoopMutex;
	trainingHudTick_t trainingHudTick = nullptr;
	getTrainingHudArgument_t getTrainingHudArgument = nullptr;
	actualGameLoop_t orig_actualGameLoop = nullptr;
	std::mutex orig_actualGameLoopMutex;
	updateAnimations_t orig_updateAnimations = nullptr;
	std::mutex orig_updateAnimationsMutex;
	const char** gameDataPtr = nullptr;
	unsigned slowmoSkipCounter = 0;
	bool ignoreAllCalls = false;
	BOOL* postEffectOnPtr = nullptr;
};

extern Game game;
