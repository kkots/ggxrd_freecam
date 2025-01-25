#include "pch.h"
#include "Controls.h"
#include "memoryFunctions.h"
#include "Detouring.h"
#include <Windowsx.h>
#include "logging.h"
#include "Settings.h"
#include <hidusage.h>
#include <dinput.h>

using namespace std::literals;

Controls controls;

bool Controls::onDllMain() {

	bool error = false;

	orig_WndProc = (WndProc_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x81\xfb\x18\x02\x00\x00\x75\x0f\x83\xfd\x01\x77\x28\x5d\xb8\x44\x51\x4d\x42\x5b\xc2\x10\x00",
		"xxxxxxxxxxxxxxxxxxxxxxx",
		{ -0xA },
		&error, "WndProc");

	if (orig_WndProc) {
		if (!detouring.attach(&(PVOID&)(orig_WndProc),
			WndProcStatic,
			&orig_WndProcMutex.mutex,
			"WndProc")) return false;
	}

	orig_SetCursorVisibility = (setCursorVisibility_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x56\x8b\xf1\x8b\x06\x8b\x50\x3c\x57\xff\xd2\x83\x7c\x24\x0c\x00\x74\x42\x85\xc0\x75\x39\x8b\x8e\xb8\x04\x00\x00",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		nullptr, "SetCursorVisibility");

	if (orig_SetCursorVisibility) {
		void(HookHelp::*setCursorVisibilityHookPtr)(BOOL isVisible) = &HookHelp::setCursorVisibilityHook;
		detouring.attach(&(PVOID&)(orig_SetCursorVisibility),
			(PVOID&)setCursorVisibilityHookPtr,
			&orig_SetCursorVisibilityMutex,
			"SetCursorVisibility");
	}
	
	// ghidra sig: 69 ff fc 01 00 00 03 3d ?? ?? ?? ?? 68 10 01 00 00
	orig_ProcessInputs = (ProcessInputs_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x69\xff\xfc\x01\x00\x00\x03\x3d\x00\x00\x00\x00\x68\x10\x01\x00\x00",
		"xxxxxxxx????xxxxx",
		{ -0x7c },
		&error, "ProcessInputs");
	if (orig_ProcessInputs) {
		UWindowsClient_Joysticks = *(BYTE**)((BYTE*)orig_ProcessInputs + 0x84);
	}
	if (orig_ProcessInputs) {
		void(HookHelp::*ProcessInputsHookPtr)(float DeltaTime) = &HookHelp::ProcessInputsHook;
		detouring.attach(&(PVOID&)(orig_ProcessInputs),
			(PVOID&)ProcessInputsHookPtr,
			&orig_ProcessInputsMutex,
			"ProcessInputs");
	}
	
	inputData.keyInputs.reserve(10);

	return !error;

}

void Controls::onDllDetach() {
	if (!thisWindow) return;
	std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
	setMagnetCursorMode(thisWindow, false);
}

LRESULT CALLBACK Controls::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HookTracker hookTracker;
	LRESULT result = controls.WndProc(hWnd, message, wParam, lParam);
	return result;
}

void Controls::HookHelp::setCursorVisibilityHook(BOOL isVisible) {
	++detouring.hooksCounter;
	controls.setCursorVisibilityHook((char*)this, isVisible);
	--detouring.hooksCounter;
}

void Controls::setCursorVisibilityHook(char* thisArg, BOOL isVisible) {
	if (isVisible && magnetCursorMode) {
		isVisible = false;
	}
	if (magnetCursorMode) {
		POINT pnt;
		GetCursorPos(&pnt);
		{
			std::unique_lock<std::mutex> guard(orig_SetCursorVisibilityMutex);
			orig_SetCursorVisibility(thisArg, isVisible);
		}
		POINT newPnt;
		GetCursorPos(&newPnt);
		if (pnt.x != newPnt.x || pnt.y != newPnt.y) {
			SetCursorPos(pnt.x, pnt.y);
		}
	} else {
		std::unique_lock<std::mutex> guard(orig_SetCursorVisibilityMutex);
		orig_SetCursorVisibility(thisArg, isVisible);
	}
}

void Controls::setMagnetCursorMode(HWND hWnd, bool isOn) {
	if (lostFocus && isOn) return;
	if (isOn && !magnetCursorMode) {
		magnetCursorMode = true;
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = hWnd;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
	}
	else if (!isOn && magnetCursorMode) {
		magnetCursorMode = false;
	}
}

POINT Controls::setCursorToCenter() {
	if (!thisWindow) return POINT{0, 0};
	RECT clientRect;
	GetClientRect(thisWindow, &clientRect);

	int clientRectCenterX = (clientRect.right - clientRect.left) / 2;
	int clientRectCenterY = (clientRect.bottom - clientRect.top) / 2;

	POINT clientPnt{ clientRectCenterX, clientRectCenterY };

	POINT screenPnt = clientPnt;
	ClientToScreen(thisWindow, &screenPnt);
	SetCursorPos(screenPnt.x, screenPnt.y);

	return clientPnt;
}

LRESULT Controls::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	thisWindow = hWnd;
	switch (message)
	{
	case WM_MOUSEWHEEL:
	{
		int wheelDelta = ((long)wParam >> 16);
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.wheelDelta += wheelDelta;
		break;
	}
	case WM_INPUT:
	{
		if (magnetCursorMode) {
			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpb[sizeof(RAWINPUT)];
			if (-1 == GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)))
				break;
			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int xPosRelative = raw->data.mouse.lLastX;
				int yPosRelative = raw->data.mouse.lLastY;
				std::unique_lock<std::mutex> guard(inputDataMutex);
				inputData.dx += xPosRelative;
				inputData.dy += yPosRelative;
				setCursorToCenter();
			}
		}
		break;
	}
	case WM_KILLFOCUS:
	{
		std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
		lostFocus = true;
		setMagnetCursorMode(hWnd, false);
		break;
	}
	case WM_SETFOCUS:
	{
		lostFocus = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, VK_LBUTTON });
		break;
	}
	case WM_LBUTTONUP:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, VK_LBUTTON });
		break;
	}
	case WM_RBUTTONDOWN:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, VK_RBUTTON });
		break;
	}
	case WM_RBUTTONUP:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, VK_RBUTTON });
		break;
	}
	case WM_MBUTTONDOWN:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, VK_MBUTTON });
		break;
	}
	case WM_MBUTTONUP:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, VK_MBUTTON });
		break;
	}
	case WM_SYSKEYDOWN:
	{
		if ((lParam & 0x40000000) == 0) {
			// key down for the first time of the repeat sequence
			std::unique_lock<std::mutex> guard(inputDataMutex);
			inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, (int)wParam });
		}
		break;
	}
	case WM_KEYDOWN:
	{
		if ((lParam & 0x40000000) == 0) {
			// key down for the first time of the repeat sequence
			std::unique_lock<std::mutex> guard(inputDataMutex);
			inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, (int)wParam });
		}
		break;
	}
	case WM_SYSKEYUP:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, (int)wParam });
		break;
	}
	case WM_KEYUP:
	{
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, (int)wParam });
		break;
	}
	case WM_DESTROY:
	{
		std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
		setMagnetCursorMode(hWnd, false);
		break;
	}
	}
	RecursiveMutexGuard guard(orig_WndProcMutex);
	return orig_WndProc(hWnd, message, wParam, lParam);
}

bool Controls::isMagnetCursorMode() {
	return magnetCursorMode;
}

void Controls::getInputData(InputData& destination) {
	std::unique_lock<std::mutex> guard(inputDataMutex);
	destination.dx += inputData.dx;
	destination.dy += inputData.dy;
	destination.keyInputs.insert(destination.keyInputs.end(), inputData.keyInputs.begin(), inputData.keyInputs.end());
	destination.wheelDelta += inputData.wheelDelta;
	destination.leftStickX = inputData.leftStickX;
	destination.leftStickY = inputData.leftStickY;
	destination.ps4RightStickX = inputData.ps4RightStickX;
	destination.ps4RightStickY = inputData.ps4RightStickY;
	destination.xboxTypeSRightStickX = inputData.xboxTypeSRightStickX;
	destination.xboxTypeSRightStickY = inputData.xboxTypeSRightStickY;
	destination.ps4LeftShoulder = inputData.ps4LeftShoulder;
	destination.ps4RightShoulder = inputData.ps4RightShoulder;
	destination.dpad = inputData.dpad;
	inputData.clear();
}

bool Controls::toggleMagnetCursorMode() {
	if (!thisWindow) return false;
	std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
	setMagnetCursorMode(thisWindow, !magnetCursorMode);
	return magnetCursorMode;
}

void Controls::HookHelp::ProcessInputsHook(float DeltaTime) {
	controls.ProcessInputsHook((BYTE*)this, DeltaTime);
}

void Controls::ProcessInputsHook(BYTE* thisArg, float DeltaTime) {
	
	static DIJOYSTATE2 oldState { 0 };
	
	{
		std::unique_lock<std::mutex> guard(orig_ProcessInputsMutex);
		orig_ProcessInputs((char*)thisArg, DeltaTime);
	}
	
	int ArrayNum = *(int*)(UWindowsClient_Joysticks + 4);
	BYTE* FJoystickInfo = *(BYTE**)UWindowsClient_Joysticks;
	while (ArrayNum >= 0) {
		if (*(void**)FJoystickInfo != nullptr  // LPDIRECTINPUTDEVICE8W DirectInput8Joystick
				&& *(BOOL*)(FJoystickInfo + 0x1f0)) {  // BOOL bIsConnected
			break;
		}
		
		FJoystickInfo += 0x1fc;
		--ArrayNum;
	}
	
	if (ArrayNum == 0) {
		std::unique_lock<std::mutex> guard(inputDataMutex);
		inputData.leftStickX = 32767;
		inputData.leftStickY = 32767;
		inputData.ps4RightStickX = 32767;
		inputData.ps4RightStickY = 32767;
		inputData.xboxTypeSRightStickX = 32767;
		inputData.xboxTypeSRightStickY = 32767;
		inputData.ps4LeftShoulder = 0;
		inputData.ps4RightShoulder = 0;
		inputData.dpad = -1;
		for (int i = 0; i < 16; ++i) {
			if (oldState.rgbButtons[i]) {
				inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, JOY_BTN_0 + i });
				oldState.rgbButtons[i] = 0;
			}
		}
		return;
	}
	
	DIJOYSTATE2* PreviousState = (DIJOYSTATE2*)(FJoystickInfo + 0xd0);
	
	std::unique_lock<std::mutex> guard(inputDataMutex);
	inputData.leftStickX = PreviousState->lX;
	inputData.leftStickY = PreviousState->lY;
	inputData.ps4RightStickX = PreviousState->lZ;
	inputData.ps4RightStickY = PreviousState->lRz;
	inputData.xboxTypeSRightStickX = PreviousState->lRx;
	inputData.xboxTypeSRightStickY = PreviousState->lRy;
	inputData.ps4LeftShoulder = PreviousState->lRx;
	inputData.ps4RightShoulder = PreviousState->lRy;
	inputData.dpad = PreviousState->rgdwPOV[0];
	for (int i = 0; i < 16; ++i) {
		if (!oldState.rgbButtons[i] && PreviousState->rgbButtons[i]) {
			inputData.keyInputs.push_back({ KEY_INPUT_TYPE_PRESSED, JOY_BTN_0 + i });
			oldState.rgbButtons[i] = PreviousState->rgbButtons[i];
		} else if (oldState.rgbButtons[i] && !PreviousState->rgbButtons[i]) {
			inputData.keyInputs.push_back({ KEY_INPUT_TYPE_RELEASED, JOY_BTN_0 + i });
			oldState.rgbButtons[i] = PreviousState->rgbButtons[i];
		}
	}
}
