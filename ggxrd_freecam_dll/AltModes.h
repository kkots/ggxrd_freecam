#pragma once
class AltModes
{
public:
	bool onDllMain();
	bool isPaused();
private:
	char* roundendCameraFlybyTypeRef = nullptr;
	char* versusModeMenuOpenRef = nullptr;
	char** pauseMenu = nullptr;
};

extern AltModes altModes;
