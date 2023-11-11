#pragma once

class Hud
{
public:
	bool onDllMain();
	void onDllDetach();
	void changeHudVisibility(bool isVisible);
	bool hideHudMode = false;
private:
	void setHidden(bool hidden);
	char** hudPtr = nullptr;
	char** globalVar = nullptr;
};

extern Hud hud;
