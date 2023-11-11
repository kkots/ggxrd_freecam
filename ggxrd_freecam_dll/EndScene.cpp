#include "pch.h"
#include "EndScene.h"
#include "Direct3DVTable.h"
#include "Detouring.h"
#include "AltModes.h"
#include "logging.h"
#include "Game.h"
#include "Camera.h"
#include <algorithm>
#include "memoryFunctions.h"
#include "Hud.h"
#include "Keyboard.h"
#include "Settings.h"

EndScene endScene;

bool EndScene::onDllMain() {
	bool error = false;

	orig_EndScene = (EndScene_t)direct3DVTable.getDirect3DVTable()[42];
	orig_Present = (Present_t)direct3DVTable.getDirect3DVTable()[17];

	// there will actually be a deadlock during DLL unloading if we don't put Present first and EndScene second

	if (!detouring.attach(&(PVOID&)(orig_Present),
		hook_Present,
		&orig_PresentMutex,
		"Present")) return false;

	if (!detouring.attach(&(PVOID&)(orig_EndScene),
		hook_EndScene,
		&orig_EndSceneMutex,
		"EndScene")) return false;

	inputData.keyInputs.reserve(10);
	tempPressedKeys.reserve(10);

	return !error;
}

HRESULT __stdcall hook_EndScene(IDirect3DDevice9* device) {
	++detouring.hooksCounter;
	if (endScene.consumePresentFlag()) {

		if (*aswEngine != nullptr) {
			endScene.endSceneHook(device);
		} else {
			endScene.endSceneOnlyProcessKeys();
			endScene.finishProcessingKeys();
		}
	}
	HRESULT result;
	{
		std::unique_lock<std::mutex> guard(endScene.orig_EndSceneMutex);
		result = endScene.orig_EndScene(device);
	}
	--detouring.hooksCounter;
	return result;
}

HRESULT __stdcall hook_Present(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
	++detouring.hooksCounter;
	HRESULT result = endScene.presentHook(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	--detouring.hooksCounter;
	return result;
}

HRESULT EndScene::presentHook(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
	setPresentFlag();
	std::unique_lock<std::mutex> guard(endScene.orig_PresentMutex);
	HRESULT result = orig_Present(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);  // may call d3d9.dll::EndScene() (and, consecutively, the hook)
	return result;
}

bool EndScene::consumePresentFlag() {
	if (!presentCalled) return false;
	presentCalled = false;
	return true;
}

void EndScene::setPresentFlag() {
	presentCalled = true;
}

bool EndScene::endSceneOnlyProcessKeys() {
	keyboard.beginUpdateStatuses();
	controls.getInputData(inputData);
	if (inputData.dx > 0) {
		tempPressAKey(MOUSE_MOVE_RIGHT);
	}
	if (inputData.dx < 0) {
		tempPressAKey(MOUSE_MOVE_LEFT);
	}
	if (inputData.dy > 0) {
		tempPressAKey(MOUSE_MOVE_DOWN);
	}
	if (inputData.dy < 0) {
		tempPressAKey(MOUSE_MOVE_UP);
	}
	if (inputData.wheelDelta > 0) {
		tempPressAKey(MOUSE_WHEEL_UP);
	}
	if (inputData.wheelDelta < 0) {
		tempPressAKey(MOUSE_WHEEL_DOWN);
	}
	for (auto it = inputData.keyInputs.begin(); it != inputData.keyInputs.end(); ++it) {
		if (it->type == KEY_INPUT_TYPE_PRESSED) {
			keyboard.pressKey(it->virtualKeyCode);
		}
		else {
			keyboard.releaseKey(it->virtualKeyCode);
		}
	}

	if (keyboard.combinationGotPressed(settings.toggleHud)) {
		hud.hideHudMode = !hud.hideHudMode;
		logwrap(fprintf(logfile, "Pressed toggleHud. hideHudMode is %d\n", (int)hud.hideHudMode));
	}
	bool obtainedMagnetCursorModeValue = false;
	if (keyboard.combinationGotPressed(settings.toggleFreecamButLeaveLocked)) {
		camera.isFreecamMode = true;
		magnetCursorMode = controls.toggleMagnetCursorMode();
		obtainedMagnetCursorModeValue = true;
		logwrap(fprintf(logfile, "Pressed toggleFreecamButLeaveLocked. magnetCursorMode is %d\n", (int)magnetCursorMode));
	}
	if (keyboard.combinationGotPressed(settings.toggleFreecam)) {
		magnetCursorMode = controls.toggleMagnetCursorMode();
		obtainedMagnetCursorModeValue = true;
		camera.isFreecamMode = magnetCursorMode;
		logwrap(fprintf(logfile, "Pressed toggleFreecam. magnetCursorMode is %d\n", (int)magnetCursorMode));
	}
	if (!obtainedMagnetCursorModeValue) {
		magnetCursorMode = controls.isMagnetCursorMode();
	}

	return magnetCursorMode;
}

void EndScene::endSceneHook(IDirect3DDevice9* device) {
	logOnce(fputs("endSceneHook called\n", logfile));

	magnetCursorMode = endSceneOnlyProcessKeys();
	if (magnetCursorMode) {
		CameraMoveInputs cameraInputs;
		cameraInputs.lookRightSpeed = keyboard.calculateCombinationSpeed(settings.lookRight, inputData, MULTIPLICATION_GOAL_LOOK) * settings.lookaroundSpeedMultiplier;
		cameraInputs.lookLeftSpeed = keyboard.calculateCombinationSpeed(settings.lookLeft, inputData, MULTIPLICATION_GOAL_LOOK) * settings.lookaroundSpeedMultiplier;
		cameraInputs.lookUpSpeed = keyboard.calculateCombinationSpeed(settings.lookUp, inputData, MULTIPLICATION_GOAL_LOOK) * settings.lookaroundSpeedMultiplier;
		cameraInputs.lookDownSpeed = keyboard.calculateCombinationSpeed(settings.lookDown, inputData, MULTIPLICATION_GOAL_LOOK) * settings.lookaroundSpeedMultiplier;
		cameraInputs.rollLeftSpeed = keyboard.calculateCombinationSpeed(settings.rollLeft, inputData, MULTIPLICATION_GOAL_ROLL) * settings.rollMultiplier;
		cameraInputs.rollRightSpeed = keyboard.calculateCombinationSpeed(settings.rollRight, inputData, MULTIPLICATION_GOAL_ROLL) * settings.rollMultiplier;
		cameraInputs.needResetRollFov = keyboard.combinationGotPressed(settings.resetRollFov);
		cameraInputs.cameraMoveForwardSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraForward, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.cameraMoveBackwardsSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraBackwards, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.cameraMoveLeftSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraLeft, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.cameraMoveRightSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraRight, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.cameraMoveUpSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraUp, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.cameraMoveDownSpeed = keyboard.calculateCombinationSpeed(settings.moveCameraDown, inputData, MULTIPLICATION_GOAL_CAMERA_MOVE) * settings.movementSpeedMultiplier;
		cameraInputs.decreaseFovSpeed = keyboard.calculateCombinationSpeed(settings.fovDecrease, inputData, MULTIPLICATION_GOAL_CHANGE_FOV) * settings.fovChangeSpeedMultiplier;
		cameraInputs.increaseFovSpeed = keyboard.calculateCombinationSpeed(settings.fovIncrease, inputData, MULTIPLICATION_GOAL_CHANGE_FOV) * settings.fovChangeSpeedMultiplier;
		logwrap(fprintf(logfile, "increaseFovSpeed: %f, settings.fovChangeSpeedMultiplier: %f\n", cameraInputs.increaseFovSpeed, settings.fovChangeSpeedMultiplier));
		
		bool isMoving = cameraInputs.cameraMoveForwardSpeed != 0.F
			|| cameraInputs.cameraMoveBackwardsSpeed != 0.F
			|| cameraInputs.cameraMoveRightSpeed != 0.F
			|| cameraInputs.cameraMoveLeftSpeed != 0.F
			|| cameraInputs.cameraMoveUpSpeed != 0.F
			|| cameraInputs.cameraMoveDownSpeed != 0.F;

		if (!isMoving) {
			hasBeenMovingFor = 0;
		} else {
			++hasBeenMovingFor;
			if (hasBeenMovingFor >= 300) {
				hasBeenMovingFor = 300;
			}
			if (hasBeenMovingFor > 60) {
				float factor = (float)(hasBeenMovingFor - 60) / 240.F * 9.F + 1.F;
				cameraInputs.cameraMoveForwardSpeed *= factor;
				cameraInputs.cameraMoveBackwardsSpeed *= factor;
				cameraInputs.cameraMoveRightSpeed *= factor;
				cameraInputs.cameraMoveLeftSpeed *= factor;
				cameraInputs.cameraMoveUpSpeed *= factor;
				cameraInputs.cameraMoveDownSpeed *= factor;
			}
		}

		camera.updateCameraManually(cameraInputs);
	} else {
		hasBeenMovingFor = 0;
	}
	hud.changeHudVisibility(!hud.hideHudMode);

	finishProcessingKeys();

#ifdef LOG_PATH
	didWriteOnce = true;
#endif
}

void EndScene::finishProcessingKeys() {
	releaseTempKeys();
	inputData.clear();
}

void EndScene::tempPressAKey(int code) {
	tempPressedKeys.push_back(code);
	keyboard.pressKey(code);
}

void EndScene::releaseTempKeys() {
	for (int code : tempPressedKeys) {
		keyboard.releaseKey(code);
	}
	tempPressedKeys.clear();
}
