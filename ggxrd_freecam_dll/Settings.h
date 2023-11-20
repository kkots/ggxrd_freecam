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
	void onDllDetach();
	void readSettingsIfChanged();
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
	KeyComboSet freezeGameToggle;
	KeyComboSet slowmoGameToggle;
	KeyComboSet allowNextFrame;
	float lookaroundSpeedMultiplier = 1.F;
	float movementSpeedMultiplier = 1.F;
	float rollMultiplier = 1.F;
	float fovChangeSpeedMultiplier = 1.F;
	int slowmoTimes = 3;
	KeyComboSet disableModToggle;
	KeyComboSet continuousScreenshotToggle;
	KeyComboSet blackBackgroundToggle;
	KeyComboSet hideOneOfTheSidesToggle;
	KeyComboSet screenshotBtn;
	std::string screenshotPath;
	bool allowContinuousScreenshotting = false;
	bool dontUseScreenshotTransparency = false;
	int sideToHide = 0;

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
	struct BooleanToParse {
		std::string name;
		bool* boolean = nullptr;
		bool defaultValue = false;
		bool booleanParsed = false;
	};
	struct IntegerToParse {
		std::string name;
		int* integer = nullptr;
		int defaultValue = false;
		bool integerParsed = false;
	};
	std::map<std::string, Key> keys;
	void addKey(std::string name, int code);
	void addKeyRange(char start, char end);
	void addKeyComboToParse(std::map<std::string, KeyComboToParse>& map, std::string name, KeyComboSet* keyComboSet, const char* defaultValue);
	void addNumberToParse(std::map<std::string, NumberToParse>& map, std::string name, float* number, float defaultValue);
	void addBooleanToParse(std::map<std::string, BooleanToParse>& map, std::string name, bool* boolean, bool defaultValue);
	void addIntegerToParse(std::map<std::string, IntegerToParse>& map, std::string name, int* boolean, int defaultValue);
	void readSettings();
	int findMinCommentPos(const char* buf) const;
	int findChar(const char* buf, char c) const;
	void replaceChar(char* buf, char orig, char newChar) const;
	void trim(std::string& str) const;
	std::vector<std::string> split(const std::string& str, char c) const;
	std::string parseKeyName(const char* buf);
	bool parseKeys(const char* keyName, const std::string& keyValue, KeyComboSet& keyCodes);
	bool parseNumber(const char* keyName, std::string keyValue, float& number);
	bool parseInteger(const char* keyName, const std::string& keyValue, int& integer);
	bool parseBoolean(const char* keyName, const std::string& keyValue, bool& integer);
	std::string getKeyValue(const char* buf) const;
	std::string toUppercase(std::string str) const;
	std::wstring getCurrentDirectory();
	bool getLastWriteTime(const std::wstring& path, FILETIME* fileTime);
	std::wstring settingsPath;
	bool firstSettingsParse = true;
	FILETIME lastSettingsWriteTime{ 0 };
	HANDLE directoryChangeHandle = NULL;
	bool lastCallFailedToGetTime = false;
};

extern Settings settings;
