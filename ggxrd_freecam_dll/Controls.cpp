#include "pch.h"
#include "Controls.h"
#include "memoryFunctions.h"
#include "Detouring.h"
#include <Windowsx.h>
#include "logging.h"
#include "Settings.h"
#include <hidusage.h>

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

	inputData.keyInputs.reserve(10);

	return !error;

}

void Controls::onDllDetach() {
	if (!thisWindow) return;
	std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
	setMagnetCursorMode(thisWindow, false);
}

LRESULT CALLBACK Controls::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	++detouring.hooksCounter;
	LRESULT result = controls.WndProc(hWnd, message, wParam, lParam);
	--detouring.hooksCounter;
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
	inputData.clear();
}

bool Controls::toggleMagnetCursorMode() {
	if (!thisWindow) return false;
	std::unique_lock<std::mutex> guard(magnetCursorModeMutex);
	setMagnetCursorMode(thisWindow, !magnetCursorMode);
	return magnetCursorMode;
}
