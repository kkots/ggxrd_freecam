#include "pch.h"
#include "Settings.h"
#include "logging.h"
#include "Keyboard.h"
#include <stdexcept>
#ifdef LOG_PATH
#include <list>
#endif

Settings settings;

const wchar_t* INI_FILE_NAME = L"ggxrd_freecam.ini";

bool Settings::onDllMain() {
	addKey("Backspace", VK_BACK);
	addKey("Tab", VK_TAB);
	addKey("Enter", VK_RETURN);
	addKey("PauseBreak", VK_PAUSE);
	addKey("CapsLock", VK_CAPITAL);
	addKey("Escape", VK_ESCAPE);
	addKey("Space", VK_SPACE);
	addKey("PageUp", VK_PRIOR);
	addKey("PageDown", VK_NEXT);
	addKey("End", VK_END);
	addKey("Home", VK_HOME);
	addKey("ArrowLeft", VK_LEFT);
	addKey("ArrowUp", VK_UP);
	addKey("ArrowRight", VK_RIGHT);
	addKey("ArrowDown", VK_DOWN);
	addKey("PrintScreen", VK_SNAPSHOT);
	addKey("Insert", VK_INSERT);
	addKey("Delete", VK_DELETE);
	addKey("Num0", VK_NUMPAD0);
	addKey("Num1", VK_NUMPAD1);
	addKey("Num2", VK_NUMPAD2);
	addKey("Num3", VK_NUMPAD3);
	addKey("Num4", VK_NUMPAD4);
	addKey("Num5", VK_NUMPAD5);
	addKey("Num6", VK_NUMPAD6);
	addKey("Num7", VK_NUMPAD7);
	addKey("Num8", VK_NUMPAD8);
	addKey("Num9", VK_NUMPAD9);
	addKey("NumMultiply", VK_MULTIPLY);
	addKey("NumAdd", VK_ADD);
	addKey("NumSubtract", VK_SUBTRACT);
	addKey("NumDecimal", VK_DECIMAL);
	addKey("NumDivide", VK_DIVIDE);
	addKey("F1", VK_F1);
	addKey("F2", VK_F2);
	addKey("F3", VK_F3);
	addKey("F4", VK_F4);
	addKey("F5", VK_F5);
	addKey("F6", VK_F6);
	addKey("F7", VK_F7);
	addKey("F8", VK_F8);
	addKey("F9", VK_F9);
	addKey("F10", VK_F10);
	addKey("F11", VK_F11);
	addKey("F12", VK_F12);
	addKey("NumLock", VK_NUMLOCK);
	addKey("ScrollLock", VK_SCROLL);
	addKey("Colon", VK_OEM_1);
	addKey("Plus", VK_OEM_PLUS);
	addKey("Minus", VK_OEM_MINUS);
	addKey("Comma", VK_OEM_COMMA);
	addKey("Period", VK_OEM_PERIOD);
	addKey("Slash", VK_OEM_2);
	addKey("Tilde", VK_OEM_3);
	addKey("OpenSquareBracket", VK_OEM_4);
	addKey("Backslash", VK_OEM_5);
	addKey("CloseSquareBracket", VK_OEM_6);
	addKey("Quote", VK_OEM_7);
	addKey("Backslash2", VK_OEM_102);
	addKey("LeftClick", VK_LBUTTON);
	addKey("RightClick", VK_RBUTTON);
	addKey("MiddleClick", VK_MBUTTON);
	addKey("MouseMoveLeft", MOUSE_MOVE_LEFT);
	addKey("MouseMoveUp", MOUSE_MOVE_UP);
	addKey("MouseMoveRight", MOUSE_MOVE_RIGHT);
	addKey("MouseMoveDown", MOUSE_MOVE_DOWN);
	addKey("MouseWheelUp", MOUSE_WHEEL_UP);
	addKey("MouseWheelDown", MOUSE_WHEEL_DOWN);

	addKeyRange('0', '9');
	addKeyRange('A', 'Z');

	addKey("Shift", VK_SHIFT);
	addKey("Ctrl", VK_CONTROL);
	addKey("Alt", VK_MENU);

	readSettings();

	table.set(MULTIPLICATION_WHAT_MOUSE, MULTIPLICATION_GOAL_LOOK, 35.F);
	table.set(MULTIPLICATION_WHAT_MOUSE, MULTIPLICATION_GOAL_ROLL, 35.F);
	table.set(MULTIPLICATION_WHAT_KEYBOARD, MULTIPLICATION_GOAL_LOOK, 100.F);
	table.set(MULTIPLICATION_WHAT_KEYBOARD, MULTIPLICATION_GOAL_ROLL, 200.F);
	table.set(MULTIPLICATION_WHAT_WHEEL, MULTIPLICATION_GOAL_LOOK, 3.F);
	table.set(MULTIPLICATION_WHAT_WHEEL, MULTIPLICATION_GOAL_ROLL, 3.F);

	table.set(MULTIPLICATION_WHAT_MOUSE, MULTIPLICATION_GOAL_CAMERA_MOVE, 1.F);
	table.set(MULTIPLICATION_WHAT_KEYBOARD, MULTIPLICATION_GOAL_CAMERA_MOVE, 2.F);
	table.set(MULTIPLICATION_WHAT_WHEEL, MULTIPLICATION_GOAL_CAMERA_MOVE, 0.5F);

	table.set(MULTIPLICATION_WHAT_WHEEL, MULTIPLICATION_GOAL_CHANGE_FOV, 0.04F);

	return true;
}

void Settings::addKey(std::string name, int code) {
	keys.insert({toUppercase(name), {name, code}});
}

void Settings::addKeyRange(char start, char end) {
	for (char c = start; c <= end; ++c) {
		std::string newName;
		newName.push_back(toupper(c));
		keys.insert({newName, {newName, c}});
	}
}

// controls:
// toggle freecam
// toggle freecam but leave locked
// move camera forward
// move camera backwards
// move camera right
// move camera left
// move camera up
// move camera down
// look up
// look down
// look right
// look left
// roll right
// roll left
// reset roll
// fov increase
// fov decrease
// toggleHud
// 
// These are just numbers:
// lookaround speed multiplier
// movement speed multiplier

// INI file must be placed next the the game's executable at SteamLibrary\steamapps\common\GUILTY GEAR Xrd -REVELATOR-\Binaries\Win32\ggxrd_hitbox_overlay.ini
// Example INI file content:
// gifModeToggle = Ctrl+F3
void Settings::readSettings() {

	std::map<std::string, KeyComboToParse> keyCombosToParse;
	addKeyComboToParse(keyCombosToParse, "toggleFreecam", &toggleFreecam, "LeftClick");
	addKeyComboToParse(keyCombosToParse, "toggleFreecamButLeaveLocked", &toggleFreecamButLeaveLocked, "RightClick");
	addKeyComboToParse(keyCombosToParse, "moveCameraForward", &moveCameraForward, "T");
	addKeyComboToParse(keyCombosToParse, "moveCameraBackwards", &moveCameraBackwards, "G");
	addKeyComboToParse(keyCombosToParse, "moveCameraLeft", &moveCameraLeft, "F");
	addKeyComboToParse(keyCombosToParse, "moveCameraRight", &moveCameraRight, "H");
	addKeyComboToParse(keyCombosToParse, "moveCameraUp", &moveCameraUp, "V");
	addKeyComboToParse(keyCombosToParse, "moveCameraDown", &moveCameraDown, "Space");
	addKeyComboToParse(keyCombosToParse, "lookUp", &lookUp, "MouseMoveUp");
	addKeyComboToParse(keyCombosToParse, "lookDown", &lookDown, "MouseMoveDown");
	addKeyComboToParse(keyCombosToParse, "lookRight", &lookRight, "MouseMoveRight");
	addKeyComboToParse(keyCombosToParse, "lookLeft", &lookLeft, "MouseMoveLeft");
	addKeyComboToParse(keyCombosToParse, "rollRight", &rollRight, "PageDown");
	addKeyComboToParse(keyCombosToParse, "rollLeft", &rollLeft, "PageUp");
	addKeyComboToParse(keyCombosToParse, "resetRollFov", &resetRollFov, "Home");
	addKeyComboToParse(keyCombosToParse, "fovDecrease", &fovDecrease, "MouseWheelUp");
	addKeyComboToParse(keyCombosToParse, "fovIncrease", &fovIncrease, "MouseWheelDown");
	addKeyComboToParse(keyCombosToParse, "toggleHud", &toggleHud, "F1");

	std::map<std::string, NumberToParse> numbersToParse;
	addNumberToParse(numbersToParse, "lookaroundSpeedMultiplier", &lookaroundSpeedMultiplier, 1.F);
	addNumberToParse(numbersToParse, "movementSpeedMultiplier", &movementSpeedMultiplier, 1.F);
	addNumberToParse(numbersToParse, "rollMultiplier", &movementSpeedMultiplier, 1.F);
	addNumberToParse(numbersToParse, "fovChangeSpeedMultiplier", &fovChangeSpeedMultiplier, 1.F);

	char errorString[500];
	char buf[1024];
	wchar_t currentPath[MAX_PATH];
	GetCurrentDirectoryW(_countof(currentPath), currentPath);
	wcscat_s(currentPath, L"\\");
	wcscat_s(currentPath, INI_FILE_NAME);
	logwrap(fprintf(logfile, "INI file path: %ls\n", currentPath));
	FILE* file = NULL;
	if (_wfopen_s(&file, currentPath, L"rt") || !file) {
		strerror_s(errorString, errno);
		logwrap(fprintf(logfile, "Could not open INI file: %s\n", errorString));
	} else {
		while (true) {
			if (!fgets(buf, 1023, file)) {
				if (ferror(file)) {
					strerror_s(errorString, errno);
					logwrap(fprintf(logfile, "Error reading INI file: %s\n", errorString));
					break;
				}
				break;
			}

			std::string keyName = parseKeyName(buf);
			std::string keyValue = getKeyValue(buf);
			if (!keyName.empty() && !keyValue.empty()) {
				std::string keyNameUpper = toUppercase(keyName);
				auto foundKeyCombo = keyCombosToParse.find(keyNameUpper);
				if (foundKeyCombo != keyCombosToParse.end()) {
					foundKeyCombo->second.keyComboParsed = parseKeys(foundKeyCombo->second.name.c_str(), keyValue, *foundKeyCombo->second.keyCombo);
				}
				auto foundNumber = numbersToParse.find(keyNameUpper);
				if (foundNumber != numbersToParse.end()) {
					foundNumber->second.numberParsed = parseNumber(foundNumber->second.name.c_str(), keyValue, *foundNumber->second.number);
				}
			}
			if (feof(file)) break;
		}
		fclose(file);
	}

	for (auto it = keyCombosToParse.begin(); it != keyCombosToParse.end(); ++it) {
		if (!it->second.keyComboParsed) {
			logwrap(fprintf(logfile, "%s not parsed, using default value: %s\n", it->second.name.c_str(), it->second.defaultValue));
			if (it->second.keyCombo == &moveCameraUp) {
				parseKeys("moveCameraUp", "V", moveCameraUp);
				parseKeys("moveCameraUp", "B", moveCameraUp);
				parseKeys("moveCameraUp", "N", moveCameraUp);
			} else {
				parseKeys(it->second.name.c_str(), it->second.defaultValue, *it->second.keyCombo);
			}
		}

		keyboard.addNewKeys(*it->second.keyCombo);
	}

	for (auto it = numbersToParse.begin(); it != numbersToParse.end(); ++it) {
		if (!it->second.numberParsed) {
			logwrap(fprintf(logfile, "%s not parsed, using default value: %f\n", it->second.name.c_str(), it->second.defaultValue));
			*it->second.number = it->second.defaultValue;
		}
	}
}

void Settings::addKeyComboToParse(std::map<std::string, KeyComboToParse>& map, std::string name, KeyComboSet* keyComboSet, const char* defaultValue) {
	map.insert({toUppercase(name), {name, keyComboSet, defaultValue}});
}

void Settings::addNumberToParse(std::map<std::string, NumberToParse>& map, std::string name, float* number, float defaultValue) {
	map.insert({toUppercase(name), {name, number, defaultValue}});
}

std::string Settings::toUppercase(std::string str) const {
	std::string result;
	result.reserve(str.size());
	for (char c : str) {
		result.push_back(toupper(c));
	}
	return result;
}

int Settings::findChar(const char* buf, char c) const {
	const char* ptr = buf;
	while (*ptr != '\0') {
		if (*ptr == c) return ptr - buf;
		++ptr;
	}
	return -1;
}

void Settings::trim(std::string& str) const {
	if (str.empty()) return;
	const char* strStart = &str.front();
	const char* c = strStart;
	while (*c <= 32 && *c != '\0') {
		++c;
	}
	if (*c == '\0') {
		str.clear();
		return;
	}

	const char* cEnd = strStart + str.size() - 1;
	while (cEnd >= c && *cEnd <= 32) {
		--cEnd;
	}
	if (cEnd < c) {
		str.clear();
		return;
	}

	str = std::string(c, cEnd - c + 1);
}

std::vector<std::string> Settings::split(const std::string& str, char c) const {
	std::vector<std::string> result;
	const char* strStart = &str.front();
	const char* strEnd = strStart + str.size();
	const char* prevPtr = strStart;
	const char* ptr = strStart;
	while (*ptr != '\0') {
		if (*ptr == c) {
			if (ptr > prevPtr) {
				result.emplace_back(prevPtr, ptr - prevPtr);
			} else if (ptr == prevPtr) {
				result.emplace_back();
			}
			prevPtr = ptr + 1;
		}
		++ptr;
	}
	if (prevPtr < strEnd) {
		result.emplace_back(prevPtr, strEnd - prevPtr);
	} else {
		result.emplace_back();
	}
	return result;
}

int Settings::findMinCommentPos(const char* buf) const {
	int colonPos = findChar(buf, ';');
	int hashtagPos = findChar(buf, '#');
	int minCommentPos = -1;
	if (colonPos != -1) minCommentPos = colonPos;
	if (minCommentPos == -1 || hashtagPos != -1 && minCommentPos != -1 && hashtagPos < minCommentPos) minCommentPos = hashtagPos;
	return minCommentPos;
}

std::string Settings::parseKeyName(const char* buf) {
	
	int minCommentPos = findMinCommentPos(buf);

	int equalSignPos = findChar(buf, '=');

	if (equalSignPos == -1 || minCommentPos != -1 && equalSignPos != -1 && equalSignPos > minCommentPos) return std::string{};

	std::string keyNameStr(buf, equalSignPos);
	trim(keyNameStr);

	return keyNameStr;
}

std::string Settings::getKeyValue(const char* buf) const {
	int minCommentPos = findMinCommentPos(buf);
	int equalSignPos = findChar(buf, '=');

	if (equalSignPos == -1 || minCommentPos != -1 && equalSignPos != -1 && equalSignPos > minCommentPos) return std::string{};

	const char* bufPos = buf + equalSignPos + 1;
	size_t bufLength = strlen(buf);
	if (minCommentPos != -1) bufLength -= (bufLength - minCommentPos);
	int lengthFromBufPos = buf + bufLength - bufPos;
	if (lengthFromBufPos == 0) return std::string{};
	std::string keyValue(bufPos, lengthFromBufPos);
	trim(keyValue);

	return keyValue;
}

bool Settings::parseKeys(const char* keyName, std::string keyValue, KeyComboSet& keyCodes) {

	keyCodes.beginInsert();

	#ifdef LOG_PATH
	std::list<std::string> parsedKeyNames;
	#endif
	std::vector<std::string> keyNames = split(keyValue, '+');
	for (std::string& parsedKeyName : keyNames) {
		trim(parsedKeyName);
		std::string parsedKeyNameUpper = toUppercase(parsedKeyName);
		auto foundKey = keys.find(parsedKeyNameUpper);
		if (foundKey != keys.end()) {
			#ifdef LOG_PATH
			parsedKeyNames.push_back(foundKey->second.name);
			#endif
			keyCodes.insert(foundKey->second.code);
		}
	}
	if (!keyCodes.empty()) {
		#ifdef LOG_PATH
		logwrap(fprintf(logfile, "Parsed key codes for %s: ", keyName));
		bool isFirst = true;
		for (auto it = parsedKeyNames.begin(); it != parsedKeyNames.end(); ++it) {
			if (!isFirst) {
				logwrap(fputs(", ", logfile));
			}
			isFirst = false;
			logwrap(fprintf(logfile, "%s", it->c_str()));
		}
		logwrap(fputs("\n", logfile));
		#endif
		keyCodes.endInsert();
		return true;
	}
	keyCodes.endInsert();
	return false;
}

void Settings::replaceChar(char* buf, char orig, char newChar) const {
	char* c = buf;
	while (*c != '\0') {
		if (*c == orig) {
			*c = newChar;
		}
		++c;
	}
}

bool Settings::parseNumber(const char* keyName, std::string keyValue, float& number) {

	replaceChar(&keyValue.front(), ',', '.');

	float result = 0.F;
	try {
		result = std::stof(keyValue);
	}
	catch (std::invalid_argument&) {
		logwrap(fprintf(logfile, "Error while parsing %s: invalid_argument error\n", keyName));
		return false;
	}
	catch (std::out_of_range&) {
		logwrap(fprintf(logfile, "Error while parsing %s: out_of_range error\n", keyName));
		return false;
	}

	logwrap(fprintf(logfile, "Parsed number for %s: %f\n", keyName, result));
	number = result;
	return true;
}
