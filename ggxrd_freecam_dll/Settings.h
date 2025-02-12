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
const int JOY_BTN_0 = 0x107;
const int JOY_BTN_1 = 0x108;
const int JOY_BTN_2 = 0x109;
const int JOY_BTN_3 = 0x10a;
const int JOY_BTN_4 = 0x10b;
const int JOY_BTN_5 = 0x10c;
const int JOY_BTN_6 = 0x10d;
const int JOY_BTN_7 = 0x10e;
const int JOY_BTN_8 = 0x10f;
const int JOY_BTN_9 = 0x110;
const int JOY_BTN_10 = 0x111;
const int JOY_BTN_11 = 0x112;
const int JOY_BTN_12 = 0x113;
const int JOY_BTN_13 = 0x114;
const int JOY_BTN_14 = 0x115;
const int JOY_BTN_15 = 0x116;
const int JOY_LEFT_STICK_LEFT = 0x117;  // lX, lY
const int JOY_LEFT_STICK_UP = 0x118;
const int JOY_LEFT_STICK_RIGHT = 0x119;
const int JOY_LEFT_STICK_DOWN = 0x11a;
const int JOY_DPAD_LEFT = 0x11b;
const int JOY_DPAD_UP = 0x11c;
const int JOY_DPAD_RIGHT = 0x11d;
const int JOY_DPAD_DOWN = 0x11e;
const int JOY_PS4_DUALSHOCK_LEFT_SHOULDER_PRESSURE = 0x11f;  // lRx
const int JOY_PS4_DUALSHOCK_RIGHT_SHOULDER_PRESSURE = 0x120;  // lRy
const int JOY_PS4_DUALSHOCK_RIGHT_STICK_LEFT = 0x121;  // lZ, lRz
const int JOY_PS4_DUALSHOCK_RIGHT_STICK_UP = 0x122;
const int JOY_PS4_DUALSHOCK_RIGHT_STICK_RIGHT = 0x123;
const int JOY_PS4_DUALSHOCK_RIGHT_STICK_DOWN = 0x124;
const int JOY_XBOX_TYPE_S_RIGHT_STICK_LEFT = 0x125;  // lRx, lRy
const int JOY_XBOX_TYPE_S_RIGHT_STICK_UP = 0x126;
const int JOY_XBOX_TYPE_S_RIGHT_STICK_RIGHT = 0x127;
const int JOY_XBOX_TYPE_S_RIGHT_STICK_DOWN = 0x128;

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
	KeyComboSet allowNextFrameKey;
	KeyComboSet togglePostEffectOnOff;
	KeyComboSet toggleAllowCreateParticles;
	float lookaroundSpeedMultiplier = 1.F;
	float movementSpeedMultiplier = 1.F;
	float rollMultiplier = 1.F;
	float fovChangeSpeedMultiplier = 1.F;
	int slowmoTimes = 3;
	int leftStickDeadzone = 0;
	int ps4DualshockRightStickDeadzone = 0;
	int xboxTypeSRightStickDeadzone = 0;
	KeyComboSet disableModToggle;
	KeyComboSet continuousScreenshotToggle;
	KeyComboSet blackBackgroundToggle;
	KeyComboSet hideOneOfTheSidesToggle;
	KeyComboSet screenshotBtn;
	std::string screenshotPath;
	bool allowContinuousScreenshotting = false;
	bool dontUseScreenshotTransparency = false;
	bool turnOffPostEffectWhenMakingBackgroundBlack = true;
	bool allowCreateParticles = true;
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
	int findCharRevW(const wchar_t* buf, wchar_t c) const;
	void replaceChar(char* buf, char orig, char newChar) const;
	void trim(std::string& str) const;
	std::vector<std::string> split(const std::string& str, char c) const;
	std::string parseKeyName(const char* buf);
	bool parseKeys(const char* keyName, const std::string& keyValue, KeyComboSet& keyCodes);
	bool parseNumber(const char* keyName, std::string keyValue, float& number);
	bool parseInteger(const char* keyName, const std::string& keyValue, int& integer);
	bool parseBoolean(const char* keyName, const std::string& keyValue, bool& integer);
	std::string getKeyValue(const char* buf) const;
	std::string toUppercase(const std::string& str) const;
	std::wstring toUppercase(const std::wstring& str) const;
	void toUppercaseInPlace(std::wstring& str) const;
	std::wstring getCurrentDirectory();
	std::wstring settingsPathGGExe;
	std::wstring settingsPathModDllDir;
	bool firstSettingsParse = true;
	FILETIME lastSettingsWriteTime{ 0 };
	HANDLE directoryChangeHandle = NULL;
	HANDLE directoryChangeHandle2 = NULL;
	std::wstring modDir;
};

extern Settings settings;
