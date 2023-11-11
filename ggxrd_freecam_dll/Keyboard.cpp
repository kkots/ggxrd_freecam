#include "pch.h"
#include "Keyboard.h"
#include "Settings.h"

Keyboard keyboard;

void Keyboard::addNewKeys(const KeyComboSet& keyCodes) {
	for (const std::vector<int>& vec : keyCodes.data) {
		for (int code : vec) {
			auto found = statuses.end();
			for (auto it = statuses.begin(); it != statuses.end(); ++it) {
				if (it->code == code) {
					found = it;
					break;
				}
			}
			if (found == statuses.end()) {
				statuses.push_back(KeyStatus{ code, false, false });
			}
		}
	}
}

bool Keyboard::isModifierKey(int code) const {
	return code == VK_SHIFT
		|| code == VK_CONTROL
		|| code == VK_MENU;
}

Keyboard::KeyStatus* Keyboard::getStatus(int code) {
	for (KeyStatus& status : statuses) {
		if (status.code == code) {
			return &status;
		}
	}
	return nullptr;
}

void Keyboard::pressKey(int code) {
	KeyStatus* found = getStatus(code);
	if (!found) return;
	if (!found->isPressed) {
		found->isPressed = true;
		found->gotPressed = true;
	}
}

void Keyboard::releaseKey(int code) {
	KeyStatus* found = getStatus(code);
	if (!found) return;
	found->isPressed = false;
}

void Keyboard::beginUpdateStatuses() {
	for (KeyStatus& status : statuses) {
		status.gotPressed = false;
	}
}

bool Keyboard::combinationIsHeld(const KeyComboSet& keyCodes) {
	if (keyCodes.empty()) return false;
	for (const std::vector<int>& vec : keyCodes.data) {
		for (int code : vec) {
			KeyStatus* found = getStatus(code);
			if (!found) break;
			if (!found->isPressed) break;
		}
		return true;
	}
	return false;
}

bool Keyboard::combinationGotPressed(const KeyComboSet& keyCodes) {
	for (const std::vector<int>& vec : keyCodes.data) {
		bool hasNonModifierKeys = false;
		for (int code : vec) {
			if (!isModifierKey(code)) {
				hasNonModifierKeys = true;
				break;
			}
		}
		bool atLeastOneGotPressed = false;
		for (int code : vec) {
			KeyStatus* found = getStatus(code);
			if (!found || !found->isPressed) {
				atLeastOneGotPressed = false;
				break;
			}
			if (!found->gotPressed) continue;
			if (hasNonModifierKeys) {
				if (!isModifierKey(code)) {
					atLeastOneGotPressed = true;
				}
			} else {
				atLeastOneGotPressed = true;
			}
		}
		if (atLeastOneGotPressed) return true;
	}
	return false;
}

float Keyboard::calculateCombinationSpeed(const KeyComboSet& keyCodes, InputData& inputData, MultiplicationGoal goal) {
	if (keyCodes.empty()) return 0.F;
	for (const std::vector<int> vec : keyCodes.data) {
		int wheelDirCode = 0;
		int mouseDirCode = 0;
		bool needToSkipThisOne = false;
		for (int code : vec) {
			KeyStatus* found = getStatus(code);
			if (!found || !found->isPressed) {
				needToSkipThisOne = true;
				break;
			}
			if (code == MOUSE_MOVE_DOWN
					|| code == MOUSE_MOVE_UP
					|| code == MOUSE_MOVE_RIGHT
					|| code == MOUSE_MOVE_LEFT) {
				mouseDirCode = code;
			} else if (code == MOUSE_WHEEL_UP || code == MOUSE_WHEEL_DOWN) {
				wheelDirCode = code;
			}
		}

		if (needToSkipThisOne) continue;

		if (mouseDirCode) {
			int value = 0;
			switch (mouseDirCode) {
			case MOUSE_MOVE_DOWN:
				if (inputData.dy > 0) value = inputData.dy;
				break;
			case MOUSE_MOVE_UP:
				if (inputData.dy < 0) value = -inputData.dy;
				break;
			case MOUSE_MOVE_RIGHT:
				if (inputData.dx > 0) value = inputData.dx;
				break;
			case MOUSE_MOVE_LEFT:
				if (inputData.dx < 0) value = -inputData.dx;
				break;
			}

			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_MOUSE, goal);

		} else if (wheelDirCode) {

			int value = 0;
			switch (wheelDirCode) {
			case MOUSE_WHEEL_UP:
				if (inputData.wheelDelta > 0) value = inputData.wheelDelta;
				break;
			case MOUSE_WHEEL_DOWN:
				if (inputData.wheelDelta < 0) value = -inputData.wheelDelta;
				break;
			}

			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_WHEEL, goal);

		} else {
			return settings.table.get(MULTIPLICATION_WHAT_KEYBOARD, goal);
		}
	}
	return 0.F;
}
