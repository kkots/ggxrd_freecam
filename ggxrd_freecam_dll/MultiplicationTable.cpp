#include "pch.h"
#include "MultiplicationTable.h"

MultiplicationTable::MultiplicationTable() {
	data.resize(MULTIPLICATION_WHAT_LAST);
	for (std::vector<float>& vec : data) {
		vec.insert(vec.begin(), MULTIPLICATION_GOAL_LAST, 1.F);
	}
}

void MultiplicationTable::set(MultiplicationWhat what, MultiplicationGoal goal, float multiplier) {
	data[what][goal] = multiplier;
}

float MultiplicationTable::get(MultiplicationWhat what, MultiplicationGoal goal) const {
	return data[what][goal];
}
