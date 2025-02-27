#pragma once
#include "RecursiveMutex.h"
#include <mutex>
#include <vector>
#include "InputData.h"
#include <chrono>

using WndProc_t = LRESULT(CALLBACK*)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
using setCursorVisibility_t = void(__thiscall*)(char* thisArg, BOOL isVisible);
using ProcessInputs_t = void(__thiscall*)(char* thisArg, float DeltaTime);

class Controls
{
public:
	bool onDllMain();
	void onDllDetach();
	bool toggleMagnetCursorMode();
	bool isMagnetCursorMode();
	void getInputData(InputData& destination);
private:
	class HookHelp {
		friend class Controls;
		void setCursorVisibilityHook(BOOL isVisible);
		void ProcessInputsHook(float DeltaTime);
	};
	LRESULT static CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void setMagnetCursorMode(HWND hWnd, bool isOn);
	void setCursorVisibilityHook(char* thisArg, BOOL isVisible);
	POINT setCursorToCenter();
	void ProcessInputsHook(BYTE* thisArg, float DeltaTime);
	std::mutex magnetCursorModeMutex;
	bool magnetCursorMode = false;

	WndProc_t orig_WndProc = nullptr;
	RecursiveMutex orig_WndProcMutex;
	setCursorVisibility_t orig_SetCursorVisibility = nullptr;
	std::mutex orig_SetCursorVisibilityMutex;
	
	std::mutex inputDataMutex;
	InputData inputData;
	bool lostFocus = false;
	
	ProcessInputs_t orig_ProcessInputs = nullptr;
	std::mutex orig_ProcessInputsMutex;
	BYTE* UWindowsClient_Joysticks = nullptr;

	HWND thisWindow = NULL;
};

extern Controls controls;
