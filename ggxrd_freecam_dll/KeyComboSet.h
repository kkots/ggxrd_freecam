#pragma once
#include <vector>
struct KeyComboSet {
	std::vector<std::vector<int>> data;
	void beginInsert();
	void insert(int code);
	void endInsert();
	bool empty() const;
private:
	bool insertJustBegan = false;
	bool insertModeOn = false;
};
