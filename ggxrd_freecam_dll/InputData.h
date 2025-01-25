#pragma once
#include <vector>

enum KeyInputType {
	KEY_INPUT_TYPE_PRESSED,
	KEY_INPUT_TYPE_RELEASED
};

struct KeyInput {
	KeyInputType type = KEY_INPUT_TYPE_PRESSED;
	int virtualKeyCode = 0;
};

struct InputData {
	int dx = 0;
	int dy = 0;
	int wheelDelta = 0;  // > 0 is up, < 0 is down
	std::vector<KeyInput> keyInputs;
	int leftStickX = 0;
	int leftStickY = 0;
	int ps4RightStickX = 0;
	int ps4RightStickY = 0;
	int xboxTypeSRightStickX = 0;
	int xboxTypeSRightStickY = 0;
	int ps4LeftShoulder = 0;
	int ps4RightShoulder = 0;
	int dpad = 0;
	void clear();
};
