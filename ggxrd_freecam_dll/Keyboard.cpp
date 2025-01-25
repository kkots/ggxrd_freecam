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
		bool checkFailed = false;
		for (int code : vec) {
			KeyStatus* found = getStatus(code);
			if (!found || !found->isPressed) {
				checkFailed = true;
				break;
			}
		}
		if (!checkFailed) return true;
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
		int leftStickDirCode = 0;
		int ps4RightStickDirCode = 0;
		int xboxTypeSRightStickDirCode = 0;
		int ps4ShoulderCode = 0;
		int dpadDirCode = 0;
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
			} else if (code == JOY_LEFT_STICK_DOWN
					|| code == JOY_LEFT_STICK_UP
					|| code == JOY_LEFT_STICK_RIGHT
					|| code == JOY_LEFT_STICK_LEFT) {
				leftStickDirCode = code;
			} else if (code == JOY_DPAD_DOWN
					|| code == JOY_DPAD_UP
					|| code == JOY_DPAD_RIGHT
					|| code == JOY_DPAD_LEFT) {
				dpadDirCode = code;
			} else if (code == JOY_PS4_DUALSHOCK_LEFT_SHOULDER_PRESSURE
					|| code == JOY_PS4_DUALSHOCK_RIGHT_SHOULDER_PRESSURE) {
				ps4ShoulderCode = code;
			} else if (code == JOY_PS4_DUALSHOCK_RIGHT_STICK_DOWN
					|| code == JOY_PS4_DUALSHOCK_RIGHT_STICK_UP
					|| code == JOY_PS4_DUALSHOCK_RIGHT_STICK_RIGHT
					|| code == JOY_PS4_DUALSHOCK_RIGHT_STICK_LEFT) {
				ps4RightStickDirCode = code;
			} else if (code == JOY_XBOX_TYPE_S_RIGHT_STICK_DOWN
					|| code == JOY_XBOX_TYPE_S_RIGHT_STICK_UP
					|| code == JOY_XBOX_TYPE_S_RIGHT_STICK_RIGHT
					|| code == JOY_XBOX_TYPE_S_RIGHT_STICK_LEFT) {
				xboxTypeSRightStickDirCode = code;
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
			
		} else if (leftStickDirCode) {
			
			int value = 0;
			switch (leftStickDirCode) {
			case JOY_LEFT_STICK_DOWN:
				if (inputData.leftStickY > 32767) value = inputData.leftStickY - 32767;
				break;
			case JOY_LEFT_STICK_UP:
				if (inputData.leftStickY < 32767) value = 32767 - inputData.leftStickY;
				break;
			case JOY_LEFT_STICK_RIGHT:
				if (inputData.leftStickX > 32767) value = inputData.leftStickX - 32767;
				break;
			case JOY_LEFT_STICK_LEFT:
				if (inputData.leftStickX < 32767) value = 32767 - inputData.leftStickX;
				break;
			}
			
			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_LEFT_STICK, goal);
			
		} else if (ps4RightStickDirCode) {
			
			int value = 0;
			switch (ps4RightStickDirCode) {
			case JOY_PS4_DUALSHOCK_RIGHT_STICK_DOWN:
				if (inputData.ps4RightStickY > 32767) value = inputData.ps4RightStickY - 32767;
				break;
			case JOY_PS4_DUALSHOCK_RIGHT_STICK_UP:
				if (inputData.ps4RightStickY < 32767) value = 32767 - inputData.ps4RightStickY;
				break;
			case JOY_PS4_DUALSHOCK_RIGHT_STICK_RIGHT:
				if (inputData.ps4RightStickX > 32767) value = inputData.ps4RightStickX - 32767;
				break;
			case JOY_PS4_DUALSHOCK_RIGHT_STICK_LEFT:
				if (inputData.ps4RightStickX < 32767) value = 32767 - inputData.ps4RightStickX;
				break;
			}
			
			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_PS4_RIGHT_STICK, goal);
			
		} else if (xboxTypeSRightStickDirCode) {
			
			int value = 0;
			switch (xboxTypeSRightStickDirCode) {
			case JOY_XBOX_TYPE_S_RIGHT_STICK_DOWN:
				if (inputData.xboxTypeSRightStickY > 32767) value = inputData.xboxTypeSRightStickY - 32767;
				break;
			case JOY_XBOX_TYPE_S_RIGHT_STICK_UP:
				if (inputData.xboxTypeSRightStickY < 32767) value = 32767 - inputData.xboxTypeSRightStickY;
				break;
			case JOY_XBOX_TYPE_S_RIGHT_STICK_RIGHT:
				if (inputData.xboxTypeSRightStickX > 32767) value = inputData.xboxTypeSRightStickX - 32767;
				break;
			case JOY_XBOX_TYPE_S_RIGHT_STICK_LEFT:
				if (inputData.xboxTypeSRightStickX < 32767) value = 32767 - inputData.xboxTypeSRightStickX;
				break;
			}
			
			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_XBOX_TYPE_S_RIGHT_STICK, goal);
			
		} else if (dpadDirCode && inputData.dpad != -1 && inputData.dpad != 65535) {
			
			int value = 0;
			switch (dpadDirCode) {
			case JOY_DPAD_DOWN:
				if (inputData.dpad == 22500 || inputData.dpad == 18000 || inputData.dpad == 13500) value = 1;
				break;
			case JOY_DPAD_UP:
				if (inputData.dpad == 31500 || inputData.dpad == 0 || inputData.dpad == 4500) value = 1;
				break;
			case JOY_DPAD_RIGHT:
				if (inputData.dpad == 4500 || inputData.dpad == 9000 || inputData.dpad == 13500) value = 1;
				break;
			case JOY_DPAD_LEFT:
				if (inputData.dpad == 31500 || inputData.dpad == 27000 || inputData.dpad == 22500) value = 1;
				break;
			}
			
			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_DPAD, goal);
			
		} else if (ps4ShoulderCode) {
			
			int value = 0;
			switch (ps4ShoulderCode) {
			case JOY_PS4_DUALSHOCK_LEFT_SHOULDER_PRESSURE:
				value = inputData.ps4LeftShoulder;
				break;
			case JOY_PS4_DUALSHOCK_RIGHT_SHOULDER_PRESSURE:
				value = inputData.ps4RightShoulder;
				break;
			}
			
			if (!value) continue;
			return (float)value * settings.table.get(MULTIPLICATION_WHAT_PS4_SHOULDER, goal);
			
		} else {
			return settings.table.get(MULTIPLICATION_WHAT_KEYBOARD, goal);
		}
	}
	return 0.F;
}
