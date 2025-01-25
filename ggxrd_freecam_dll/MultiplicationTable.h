#pragma once
#include <vector>

// kinda misleading name, sorry

enum MultiplicationWhat {
	MULTIPLICATION_WHAT_MOUSE = 0,
	MULTIPLICATION_WHAT_KEYBOARD = 1,
	MULTIPLICATION_WHAT_WHEEL = 2,
	MULTIPLICATION_WHAT_LEFT_STICK = 3,
	MULTIPLICATION_WHAT_PS4_RIGHT_STICK = 4,
	MULTIPLICATION_WHAT_XBOX_TYPE_S_RIGHT_STICK = 5,
	MULTIPLICATION_WHAT_PS4_SHOULDER = 6,
	MULTIPLICATION_WHAT_DPAD = 7,
	MULTIPLICATION_WHAT_LAST = 8  // Must always be last
};

enum MultiplicationGoal {
	MULTIPLICATION_GOAL_LOOK,
	MULTIPLICATION_GOAL_CAMERA_MOVE,
	MULTIPLICATION_GOAL_ROLL,
	MULTIPLICATION_GOAL_CHANGE_FOV,
	MULTIPLICATION_GOAL_LAST  // Must always be last
};

struct MultiplicationTable
{
	std::vector<std::vector<float>> data;
	MultiplicationTable();
	void set(MultiplicationWhat what, MultiplicationGoal goal, float multiplier);
	float get(MultiplicationWhat what, MultiplicationGoal goal) const;
};
