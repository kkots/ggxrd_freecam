#include "pch.h"
#include "AltModes.h"
#include "memoryFunctions.h"

AltModes altModes;

bool AltModes::onDllMain() {

	pauseMenu = (char**)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x8b\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x3c\x10\x75\x16\x8b\x15\x00\x00\x00\x00\x39\xb2\xf0\x00\x00\x00\x74\x08\xc7\x44\x24\x48\xd9\x00\x00\x00",
		"xx????x????xxxxxx????xxxxxxxxxxxxxxxx",
		{17, 0},
		NULL, "pauseMenu");

	versusModeMenuOpenRef = (char*)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\xe8\x00\x00\x00\x00\x03\xf3\x83\xfe\x66\x7c\xe5\xa1\x00\x00\x00\x01\x39\x3d\x00\x00\x00\x01\x74\x0f\x29\x1d\x00\x00\x00\x01\x83\xf8\x05\x73\x13\x03\xc3\xeb\x0a\x3b\xc7\x0f\x84\x99\x00\x00\x00\x2b\xc3",
		"x????xxxxxxxx????xx????xxxx????xxxxxxxxxxxxxxxxxxx",
		{ 19, 0 },
		NULL, "versusModeMenuOpenRef");

	return true;
}

bool AltModes::isPaused() {
	if (pauseMenu
		&& *(unsigned int*)(*pauseMenu + 0x10C) == 1
		//&& (*(unsigned int*)(*pauseMenu + 0x10) & 0x10000) == 0
		|| versusModeMenuOpenRef && *versusModeMenuOpenRef) {
		return true;
	}
	return false;
}
