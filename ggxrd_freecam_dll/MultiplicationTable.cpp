#include "pch.h"
#include "MultiplicationTable.h"

MultiplicationTable::MultiplicationTable() {
	data.resize(3);
	for (std::vector<float>& vec : data) {
		vec.insert(vec.begin(), 4, 1.F);
	}
}

void MultiplicationTable::set(MultiplicationWhat what, MultiplicationGoal goal, float multiplier) {
	data[what][goal] = multiplier;
}

float MultiplicationTable::get(MultiplicationWhat what, MultiplicationGoal goal) const {
	return data[what][goal];
}