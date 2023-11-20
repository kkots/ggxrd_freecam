#include "pch.h"
#include "KeyComboSet.h"

void KeyComboSet::beginInsert() {
	insertJustBegan = true;
	insertModeOn = true;
}

void KeyComboSet::endInsert() {
	insertJustBegan = false;
	insertModeOn = false;
}

bool KeyComboSet::empty() const {
	if (insertModeOn && insertJustBegan) return true;
	return data.empty();
}

void KeyComboSet::insert(int code) {
	if (insertModeOn && insertJustBegan) {
		data.emplace_back();
		insertJustBegan = false;
	}
	if (data.empty()) return;
	data.back().push_back(code);
}

void KeyComboSet::clear() {
	data.clear();
	insertModeOn = false;
	insertJustBegan = false;
}
