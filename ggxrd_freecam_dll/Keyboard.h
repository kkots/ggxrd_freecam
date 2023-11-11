#pragma once
#include <vector>
#include "MultiplicationTable.h"
#include "InputData.h"
#include "KeyComboSet.h"

class Keyboard
{
public:
	void pressKey(int code);
	void beginUpdateStatuses();
	void releaseKey(int code);
	bool combinationGotPressed(const KeyComboSet& keyCodes);
	float calculateCombinationSpeed(const KeyComboSet& keyCodes, InputData& inputData, MultiplicationGoal goal);
	bool combinationIsHeld(const KeyComboSet& keyCodes);
	void addNewKeys(const KeyComboSet& keyCodes);
private:
	struct KeyStatus {
		int code = 0;
		bool isPressed = false;
		bool gotPressed = false;
	};
	std::vector<KeyStatus> statuses;
	bool isModifierKey(int code) const;
	KeyStatus* getStatus(int code);
};

extern Keyboard keyboard;
