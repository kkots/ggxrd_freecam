#include "pch.h"
#include "Hud.h"
#include "memoryFunctions.h"
#include "Game.h"
#include "AltModes.h"

Hud hud;

bool Hud::onDllMain() {
	bool error = false;

	hudPtr = (char**)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x75\x1e\x8b\x4e\x14\x6a\x00\x40\x56\x89\x46\x18\xff\x15\x00\x00\x00\x00\x8b\x44\x24\x04\xa3\x00\x00\x00\x00\x5e\x59\xc2\x08\x00\x8b\x4c\x24\x04\x89\x0d\x00\x00\x00\x00\x5e\x59\xc2\x08\x00",
		"xxxxxxxxxxxxxx????xxxxx????xxxxxxxxxxx????xxxxx",
		{0x17, 0},
		&error, "hudPtr");

	globalVar = (char**)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\xe8\x00\x00\x00\x00\xf3\x0f\x10\x44\x24\x7c\x83\xc4\x10\x68\x00\x00\x00\x00\x51\xf3\x0f\x11\x04\x24\xe8\x00\x00\x00\x00\x83\xc4\x08\x8b\xcd\xe8\x00\x00\x00\x00",
		"x????xxxxxxxxxx????xxxxxxx????xxxxxx????",
		{15, 0},
		&error, "globalVar");

	return !error;
}

void Hud::onDllDetach() {
	changeHudVisibility(true);
}

void Hud::changeHudVisibility(bool isVisible) {
	if (!aswEngine || !*aswEngine || !hudPtr || !*hudPtr) return;
	unsigned int* bShowHud = (unsigned int*)(*hudPtr + 0x1D8);
	unsigned int bShowHudCurrent = *bShowHud;
	bool needToSetHidden = false;
	if (isVisible && (bShowHudCurrent & 2) == 0) {
		*bShowHud |= 2;
		needToSetHidden = true;
	} else if (!isVisible && (bShowHudCurrent & 2) != 0) {
		*bShowHud &= ~2;
		needToSetHidden = true;
	}
	bool isPaused = !game.isGameModeOnline() && altModes.isPaused();
	// When the game is paused via opening the menu, only the tension bars react to 0x2 flag.
	// Upper hud must be hidden in a few extra steps.
	if (needToSetHidden && isPaused) {
		setHidden(!isVisible);
	}
}

void Hud::setHidden(bool hidden) {
	// This is essentially what "AActorexecSetHidden" does.
	char* location = *(char**)((char*)globalVar + 0x18);
	location = *(char**)(location + 4);

	unsigned int* val = (unsigned int*)(location + 0xB0);
	if (hidden) {
		*val = *val | 00100002;
	} else {
		unsigned int valCurrent = *val;
		*val = (valCurrent & 0xFFFFFFFD) | 00100000;
	}

	val = (unsigned int*)(*(char**)(location + 0x188) + 0x50);
	*val = *val | 4;

}
