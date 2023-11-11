#pragma once
#include <vector>

// kinda misleading name, sorry

enum MultiplicationWhat {
	MULTIPLICATION_WHAT_MOUSE = 0,
	MULTIPLICATION_WHAT_KEYBOARD = 1,
	MULTIPLICATION_WHAT_WHEEL = 2
};

enum MultiplicationGoal {
	MULTIPLICATION_GOAL_LOOK,
	MULTIPLICATION_GOAL_CAMERA_MOVE,
	MULTIPLICATION_GOAL_ROLL,
	MULTIPLICATION_GOAL_CHANGE_FOV
};

struct MultiplicationTable
{
	std::vector<std::vector<float>> data;
	MultiplicationTable();
	void set(MultiplicationWhat what, MultiplicationGoal goal, float multiplier);
	float get(MultiplicationWhat what, MultiplicationGoal goal) const;
};
