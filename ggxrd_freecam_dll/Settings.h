#pragma once
#include <vector>
#include <string>
#include <map>
#include "MultiplicationTable.h"
#include "KeyComboSet.h"

const int MOUSE_MOVE_LEFT = 0x101;
const int MOUSE_MOVE_UP = 0x102;
const int MOUSE_MOVE_RIGHT = 0x103;
const int MOUSE_MOVE_DOWN = 0x104;
const int MOUSE_WHEEL_UP = 0x105;
const int MOUSE_WHEEL_DOWN = 0x106;

class Settings
{
public:
	bool onDllMain();
	KeyComboSet toggleFreecam;
	KeyComboSet toggleFreecamButLeaveLocked;
	KeyComboSet moveCameraForward;
	KeyComboSet moveCameraBackwards;
	KeyComboSet moveCameraLeft;
	KeyComboSet moveCameraRight;
	KeyComboSet moveCameraUp;
	KeyComboSet moveCameraDown;
	KeyComboSet lookUp;
	KeyComboSet lookDown;
	KeyComboSet lookRight;
	KeyComboSet lookLeft;
	KeyComboSet rollRight;
	KeyComboSet rollLeft;
	KeyComboSet resetRollFov;
	KeyComboSet fovDecrease;
	KeyComboSet fovIncrease;
	KeyComboSet toggleHud;
	float lookaroundSpeedMultiplier = 1.F;
	float movementSpeedMultiplier = 1.F;
	float rollMultiplier = 1.F;
	float fovChangeSpeedMultiplier = 1.F;

	MultiplicationTable table;
private:
	struct Key {
		std::string name;
		int code = 0;
	};
	struct KeyComboToParse {
		std::string name;
		KeyComboSet* keyCombo = nullptr;
		const char* defaultValue = nullptr;
		bool keyComboParsed = false;
	};
	struct NumberToParse {
		std::string name;
		float* number = nullptr;
		float defaultValue = 0.F;
		bool numberParsed = false;
	};
	std::map<std::string, Key> keys;
	void addKey(std::string name, int code);
	void addKeyRange(char start, char end);
	void addKeyComboToParse(std::map<std::string, KeyComboToParse>& map, std::string name, KeyComboSet* keyComboSet, const char* defaultValue);
	void addNumberToParse(std::map<std::string, NumberToParse>& map, std::string name, float* number, float defaultValue);
	void readSettings();
	int findMinCommentPos(const char* buf) const;
	int findChar(const char* buf, char c) const;
	void replaceChar(char* buf, char orig, char newChar) const;
	void trim(std::string& str) const;
	std::vector<std::string> split(const std::string& str, char c) const;
	std::string parseKeyName(const char* buf);
	bool parseKeys(const char* keyName, std::string keyValue, KeyComboSet& keyCodes);
	bool parseNumber(const char* keyName, std::string keyValue, float& number);
	std::string getKeyValue(const char* buf) const;
	std::string toUppercase(std::string str) const;
};

extern Settings settings;
