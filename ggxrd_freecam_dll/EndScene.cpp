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
#include "EntityList.h"
#include "Screenshotting.h"

EndScene endScene;

bool EndScene::onDllMain() {
	bool error = false;

	orig_EndScene = (EndScene_t)direct3DVTable.getDirect3DVTable()[42];
	orig_Present = (Present_t)direct3DVTable.getDirect3DVTable()[17];
	orig_Reset = (Reset_t)direct3DVTable.getDirect3DVTable()[16];

	// there will actually be a deadlock during DLL unloading if we don't put Present first and EndScene second

	if (!detouring.attach(&(PVOID&)(orig_Present),
		hook_Present,
		&orig_PresentMutex,
		"Present")) return false;

	if (!detouring.attach(&(PVOID&)(orig_EndScene),
		hook_EndScene,
		&orig_EndSceneMutex,
		"EndScene")) return false;

	if (!detouring.attach(
		&(PVOID&)(orig_Reset),
		hook_Reset,
		&orig_ResetMutex,
		"Reset")) return false;

	inputData.keyInputs.reserve(10);
	tempPressedKeys.reserve(10);

	return !error;
}

void EndScene::onDllDetach() {
	screenshotting.clear(true);
}

HRESULT __stdcall hook_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	++detouring.hooksCounter;
	endScene.resetHook();
	HRESULT result;
	{
		std::unique_lock<std::mutex> guard(endScene.orig_ResetMutex);
		result = endScene.orig_Reset(device, pPresentationParameters);
	}
	--detouring.hooksCounter;
	return result;
}

void EndScene::resetHook() {
	screenshotting.clear();
}

HRESULT __stdcall hook_EndScene(IDirect3DDevice9* device) {
	HookTracker hookTracker;
	if (endScene.consumePresentFlag()) {

		if (*aswEngine != nullptr) {
			endScene.endSceneHook(device);
		} else {
			endScene.endSceneOnlyProcessKeys();
			endScene.finishProcessingKeys();
			endScene.clearContinuousScreenshotMode();
		}
	}
	HRESULT result;
	{
		std::unique_lock<std::mutex> guard(endScene.orig_EndSceneMutex);
		result = endScene.orig_EndScene(device);
	}
	return result;
}

HRESULT __stdcall hook_Present(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
	HookTracker hookTracker;
	HRESULT result = endScene.presentHook(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
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
	settings.readSettingsIfChanged();	
	keyboard.beginUpdateStatuses();
	controls.getInputData(inputData);
	if (inputData.dx > 0) {
		tempPressKey(MOUSE_MOVE_RIGHT);
	}
	if (inputData.dx < 0) {
		tempPressKey(MOUSE_MOVE_LEFT);
	}
	if (inputData.dy > 0) {
		tempPressKey(MOUSE_MOVE_DOWN);
	}
	if (inputData.dy < 0) {
		tempPressKey(MOUSE_MOVE_UP);
	}
	if (inputData.wheelDelta > 0) {
		tempPressKey(MOUSE_WHEEL_UP);
	}
	if (inputData.wheelDelta < 0) {
		tempPressKey(MOUSE_WHEEL_DOWN);
	}
	if (inputData.dpad != -1 && inputData.dpad != 65535) {
		if (inputData.dpad == 0) {
			tempPressKey(JOY_DPAD_UP);
		} else if (inputData.dpad == 4500) {
			tempPressKey(JOY_DPAD_UP);
			tempPressKey(JOY_DPAD_RIGHT);
		} else if (inputData.dpad == 9000) {
			tempPressKey(JOY_DPAD_RIGHT);
		} else if (inputData.dpad == 13500) {
			tempPressKey(JOY_DPAD_RIGHT);
			tempPressKey(JOY_DPAD_DOWN);
		} else if (inputData.dpad == 18000) {
			tempPressKey(JOY_DPAD_DOWN);
		} else if (inputData.dpad == 22500) {
			tempPressKey(JOY_DPAD_DOWN);
			tempPressKey(JOY_DPAD_LEFT);
		} else if (inputData.dpad == 27000) {
			tempPressKey(JOY_DPAD_LEFT);
		} else if (inputData.dpad == 31500) {
			tempPressKey(JOY_DPAD_LEFT);
			tempPressKey(JOY_DPAD_UP);
		}
	}
	
	if (inputData.leftStickX >= 32767 - settings.leftStickDeadzone
			&& inputData.leftStickX <= 32767 + settings.leftStickDeadzone) {
		inputData.leftStickX = 32767;
	}
	if (inputData.leftStickY >= 32767 - settings.leftStickDeadzone
			&& inputData.leftStickY <= 32767 + settings.leftStickDeadzone) {
		inputData.leftStickY = 32767;
	}
	
	if (inputData.ps4RightStickX >= 32767 - settings.ps4DualshockRightStickDeadzone
			&& inputData.ps4RightStickX <= 32767 + settings.ps4DualshockRightStickDeadzone) {
		inputData.ps4RightStickX = 32767;
	}
	if (inputData.ps4RightStickY >= 32767 - settings.ps4DualshockRightStickDeadzone
			&& inputData.ps4RightStickY <= 32767 + settings.ps4DualshockRightStickDeadzone) {
		inputData.ps4RightStickY = 32767;
	}
	
	if (inputData.xboxTypeSRightStickX >= 32767 - settings.xboxTypeSRightStickDeadzone
			&& inputData.xboxTypeSRightStickX <= 32767 + settings.xboxTypeSRightStickDeadzone) {
		inputData.xboxTypeSRightStickX = 32767;
	}
	if (inputData.xboxTypeSRightStickY >= 32767 - settings.xboxTypeSRightStickDeadzone
			&& inputData.xboxTypeSRightStickY <= 32767 + settings.xboxTypeSRightStickDeadzone) {
		inputData.xboxTypeSRightStickY = 32767;
	}
	
	if (inputData.leftStickX > 32767) {
		tempPressKey(JOY_LEFT_STICK_RIGHT);
	}
	if (inputData.leftStickX < 32767) {
		tempPressKey(JOY_LEFT_STICK_LEFT);
	}
	if (inputData.leftStickY > 32767) {
		tempPressKey(JOY_LEFT_STICK_DOWN);
	}
	if (inputData.leftStickY < 32767) {
		tempPressKey(JOY_LEFT_STICK_UP);
	}
	
	if (inputData.ps4RightStickX > 32767) {
		tempPressKey(JOY_PS4_DUALSHOCK_RIGHT_STICK_RIGHT);
	}
	if (inputData.ps4RightStickX < 32767) {
		tempPressKey(JOY_PS4_DUALSHOCK_RIGHT_STICK_LEFT);
	}
	if (inputData.ps4RightStickY > 32767) {
		tempPressKey(JOY_PS4_DUALSHOCK_RIGHT_STICK_DOWN);
	}
	if (inputData.ps4RightStickY < 32767) {
		tempPressKey(JOY_PS4_DUALSHOCK_RIGHT_STICK_UP);
	}
	
	if (inputData.xboxTypeSRightStickX > 32767) {
		tempPressKey(JOY_XBOX_TYPE_S_RIGHT_STICK_RIGHT);
	}
	if (inputData.xboxTypeSRightStickX < 32767) {
		tempPressKey(JOY_XBOX_TYPE_S_RIGHT_STICK_LEFT);
	}
	if (inputData.xboxTypeSRightStickY > 32767) {
		tempPressKey(JOY_XBOX_TYPE_S_RIGHT_STICK_DOWN);
	}
	if (inputData.xboxTypeSRightStickY < 32767) {
		tempPressKey(JOY_XBOX_TYPE_S_RIGHT_STICK_UP);
	}
	
	if (inputData.ps4LeftShoulder) {
		tempPressKey(JOY_PS4_DUALSHOCK_LEFT_SHOULDER_PRESSURE);
	}
	if (inputData.ps4RightShoulder) {
		tempPressKey(JOY_PS4_DUALSHOCK_RIGHT_SHOULDER_PRESSURE);
	}
	
	for (auto it = inputData.keyInputs.begin(); it != inputData.keyInputs.end(); ++it) {
		if (it->type == KEY_INPUT_TYPE_PRESSED) {
			keyboard.pressKey(it->virtualKeyCode);
		}
		else {
			keyboard.releaseKey(it->virtualKeyCode);
		}
	}

	bool obtainedMagnetCursorModeValue = false;
	if (keyboard.combinationGotPressed(settings.disableModToggle)) {
		if (!modDisabled) {
			modDisabled = true;
			logwrap(fputs("Mod disabled\n", logfile));
		} else {
			modDisabled = false;
			logwrap(fputs("Mod enabled\n", logfile));
		}
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.toggleHud)) {
		hud.hideHudMode = !hud.hideHudMode;
		logwrap(fprintf(logfile, "Pressed toggleHud. hideHudMode is %d\n", (int)hud.hideHudMode));
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.toggleFreecamButLeaveLocked)) {
		camera.isFreecamMode = true;
		magnetCursorMode = controls.toggleMagnetCursorMode();
		obtainedMagnetCursorModeValue = true;
		logwrap(fprintf(logfile, "Pressed toggleFreecamButLeaveLocked. magnetCursorMode is %d\n", (int)magnetCursorMode));
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.toggleFreecam)) {
		magnetCursorMode = controls.toggleMagnetCursorMode();
		obtainedMagnetCursorModeValue = true;
		camera.isFreecamMode = magnetCursorMode;
		logwrap(fprintf(logfile, "Pressed toggleFreecam. magnetCursorMode is %d\n", (int)magnetCursorMode));
	}
	if (!obtainedMagnetCursorModeValue) {
		magnetCursorMode = controls.isMagnetCursorMode();
	}

	GameMode gameMode = game.getGameMode();
	bool trainingMode = gameMode == GAME_MODE_TRAINING;
	bool replayMode = gameMode == GAME_MODE_REPLAY;
	if (!modDisabled && keyboard.combinationGotPressed(settings.freezeGameToggle)) {
		if (freezeGame == true) {
			freezeGame = false;
			logwrap(fputs("Freeze game turned off\n", logfile));
		}
		else {
			freezeGame = true;
			logwrap(fputs("Freeze game turned on\n", logfile));
		}
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.slowmoGameToggle)) {
		if (game.slowmoGame == true) {
			game.slowmoGame = false;
			logwrap(fputs("Slowmo game turned off\n", logfile));
		}
		else if (trainingMode) {
			game.slowmoGame = true;
			logwrap(fputs("Slowmo game turned on\n", logfile));
		}
	}
	bool allowNextFrameIsHeld = !modDisabled && keyboard.combinationIsHeld(settings.allowNextFrameKey);
	if (allowNextFrameIsHeld) {
		bool allowPress = false;
		if (allowNextFrameBeenHeldFor == 0) {
			allowPress = true;
		}
		else if (allowNextFrameBeenHeldFor >= 40) {
			allowNextFrameBeenHeldFor = 40;
			++allowNextFrameCounter;
			if (allowNextFrameCounter >= 10) {
				allowPress = true;
				allowNextFrameCounter = 0;
			}
		}
		if (trainingMode && allowPress) {
			game.allowNextFrame = true;
			logwrap(fputs("allowNextFrameKey pressed\n", logfile));
		}
		++allowNextFrameBeenHeldFor;
	}
	else {
		allowNextFrameBeenHeldFor = 0;
		allowNextFrameCounter = 0;
	}
	needToTakeScreenshot = false;
	if (!modDisabled && keyboard.combinationGotPressed(settings.screenshotBtn)) {
		needToTakeScreenshot = true;
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.continuousScreenshotToggle)) {
		if (continuousScreenshotMode) {
			continuousScreenshotMode = false;
			logwrap(fputs("Continuous screenshot mode off\n", logfile));
		}
		else if (trainingMode) {
			continuousScreenshotMode = true;
			logwrap(fputs("Continuous screenshot mode on\n", logfile));
		}
	}
	needContinuouslyTakeScreens = false;
	if (!modDisabled
			&& (keyboard.combinationIsHeld(settings.screenshotBtn) && settings.allowContinuousScreenshotting || continuousScreenshotMode)
			&& *aswEngine
			&& trainingMode
			&& !settings.screenshotPath.empty()) {
		needContinuouslyTakeScreens = true;
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.blackBackgroundToggle)) {
		if (camera.darkenMode) {
			camera.darkenMode = false;
			logwrap(fputs("Darken mode off\n", logfile));
		}
		else if (trainingMode || replayMode) {
			camera.darkenMode = true;
			logwrap(fputs("Darken mode on\n", logfile));
		}
		onGifModeBlackBackgroundChanged();
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.togglePostEffectOnOff)) {
		BOOL theValue = !game.postEffectOn();
		game.postEffectOn() = theValue;
		if (theValue) {
			logwrap(fputs("Post Effect turned on\n", logfile));
		}
		else {
			logwrap(fputs("Post Effect turned off\n", logfile));
		}
	}
	if (!modDisabled && keyboard.combinationGotPressed(settings.hideOneOfTheSidesToggle)) {
		if (hideOpponent) {
			hideOpponent = false;
			game.everyoneInvulnerable = false;
			logwrap(fputs("Hide opponent off\n", logfile));
		}
		else if (trainingMode || replayMode) {
			hideOpponent = true;
			game.everyoneInvulnerable = true;
			logwrap(fputs("Hide opponent on\n", logfile));
		}
	}
	game.freezeGame = (allowNextFrameIsHeld || freezeGame) && !modDisabled;
	if (!trainingMode || modDisabled) {
		game.slowmoGame = false;
	}
	if (modDisabled) {
		clearContinuousScreenshotMode();
		if (magnetCursorMode) {
			magnetCursorMode = controls.toggleMagnetCursorMode();
		}
		camera.isFreecamMode = false;
		camera.darkenMode = false;
		onGifModeBlackBackgroundChanged();
		hud.hideHudMode = false;
		freezeGame = false;
		hideOpponent = false;
		game.everyoneInvulnerable = false;
	}
	if (!trainingMode && !replayMode) {
		camera.darkenMode = false;
		onGifModeBlackBackgroundChanged();
		hideOpponent = false;
		game.everyoneInvulnerable = false;
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

	entityList.populate();

	noGravGifMode();

	bool frameHasChanged = false;
	unsigned int p1CurrentTimer = ~0;
	unsigned int p2CurrentTimer = ~0;
	if (!modDisabled) {
		if (entityList.count >= 1) {
			p1CurrentTimer = *(unsigned int*)(entityList.slots[0] + 0x130);
		}
		if (entityList.count >= 2) {
			p2CurrentTimer = *(unsigned int*)(entityList.slots[1] + 0x130);
		}
		if (p1CurrentTimer != p1PreviousTimeOfTakingScreen
				|| p2CurrentTimer != p2PreviousTimeOfTakingScreen) {
			frameHasChanged = true;
		}
		if (needContinuouslyTakeScreens) {
			if (frameHasChanged) {
				needToTakeScreenshot = true;
			}
			p1PreviousTimeOfTakingScreen = p1CurrentTimer;
			p2PreviousTimeOfTakingScreen = p2CurrentTimer;
		}
		else if (frameHasChanged) {
			p1PreviousTimeOfTakingScreen = ~0;
			p2PreviousTimeOfTakingScreen = ~0;
		}
	} else {
		p1PreviousTimeOfTakingScreen = ~0;
		p2PreviousTimeOfTakingScreen = ~0;
	}

	if (!modDisabled && needToTakeScreenshot) {
		screenshotting.takeScreenshot(device);
	}

#ifdef LOG_PATH
	didWriteOnce = true;
#endif
}

void EndScene::finishProcessingKeys() {
	releaseTempKeys();
	inputData.clear();
}

void EndScene::tempPressKey(int code) {
	tempPressedKeys.push_back(code);
	keyboard.pressKey(code);
}

void EndScene::releaseTempKeys() {
	for (int code : tempPressedKeys) {
		keyboard.releaseKey(code);
	}
	tempPressedKeys.clear();
}

void EndScene::clearContinuousScreenshotMode() {
	continuousScreenshotMode = false;
	p1PreviousTimeOfTakingScreen = ~0;
	p2PreviousTimeOfTakingScreen = ~0;
}

void EndScene::noGravGifMode() {
	char sideToHide;
	GameMode gameMode = game.getGameMode();
	bool trainingMode = gameMode == GAME_MODE_TRAINING;
	bool replayMode = gameMode == GAME_MODE_REPLAY;
	sideToHide = settings.sideToHide;
	if (sideToHide != 0 && sideToHide != 1) sideToHide = 1;

	bool useGifMode = hideOpponent && (trainingMode || replayMode) && !modDisabled;
	if (useGifMode) {
		for (auto it = hiddenEntities.begin(); it != hiddenEntities.end(); ++it) {
			it->wasFoundOnThisFrame = false;
		}
		for (int i = 0; i < entityList.count; ++i) {
			char* ent = entityList.list[i];
			if (*(char*)(ent + 0x40) == sideToHide) {
				const int currentScaleX = *(int*)(ent + 0x264);
				const int currentScaleY = *(int*)(ent + 0x268);
				const int currentScaleZ = *(int*)(ent + 0x26C);
				const int currentScaleDefault = *(int*)(ent + 0x2594);

				auto found = findHiddenEntity(ent);
				if (found == hiddenEntities.end()) {
					hiddenEntities.emplace_back();
					HiddenEntity& hiddenEntity = hiddenEntities.back();
					hiddenEntity.ent = ent;
					hiddenEntity.scaleX = currentScaleX;
					hiddenEntity.scaleY = currentScaleY;
					hiddenEntity.scaleZ = currentScaleZ;
					hiddenEntity.scaleDefault = currentScaleDefault;
					hiddenEntity.wasFoundOnThisFrame = true;
				}
				else {
					HiddenEntity& hiddenEntity = *found;
					if (currentScaleX != 0) {
						hiddenEntity.scaleX = currentScaleX;
					}
					if (currentScaleY != 0) {
						hiddenEntity.scaleY = currentScaleY;
					}
					if (currentScaleZ != 0) {
						hiddenEntity.scaleZ = currentScaleZ;
					}
					if (currentScaleDefault != 0) {
						hiddenEntity.scaleDefault = currentScaleDefault;
					}
					hiddenEntity.wasFoundOnThisFrame = true;
				}
				*(int*)(ent + 0x264) = 0;
				*(int*)(ent + 0x268) = 0;
				*(int*)(ent + 0x26C) = 0;
				*(int*)(ent + 0x2594) = 0;
			}
		}
		auto it = hiddenEntities.begin();
		while (it != hiddenEntities.end()) {
			if (!it->wasFoundOnThisFrame) {
				hiddenEntities.erase(it);
			}
			else {
				++it;
			}
		}
	}
	else {
		for (int i = 0; i < entityList.count; ++i) {
			char* ent = entityList.list[i];
			auto found = findHiddenEntity(ent);
			if (found != hiddenEntities.end()) {
				const int currentScaleX = *(int*)(ent + 0x264);
				const int currentScaleY = *(int*)(ent + 0x268);
				const int currentScaleZ = *(int*)(ent + 0x26C);
				const int currentScaleDefault = *(int*)(ent + 0x2594);

				if (currentScaleX == 0) {
					*(int*)(ent + 0x264) = found->scaleX;
				}
				if (currentScaleY == 0) {
					*(int*)(ent + 0x268) = found->scaleY;
				}
				if (currentScaleZ == 0) {
					*(int*)(ent + 0x26C) = found->scaleZ;
				}
				if (currentScaleDefault == 0) {
					*(int*)(ent + 0x2594) = found->scaleDefault;
				}
			}
		}
		hiddenEntities.clear();
	}
}

std::vector<EndScene::HiddenEntity>::iterator EndScene::findHiddenEntity(char* ent) {
	for (auto it = hiddenEntities.begin(); it != hiddenEntities.end(); ++it) {
		if (it->ent == ent) {
			return it;
		}
	}
	return hiddenEntities.end();
}

void EndScene::onGifModeBlackBackgroundChanged() {
	bool newIsInMode = camera.darkenMode && settings.turnOffPostEffectWhenMakingBackgroundBlack;
	if (newIsInMode != isInDarkenModePlusTurnOffPostEffect) {
		isInDarkenModePlusTurnOffPostEffect = newIsInMode;
		if (isInDarkenModePlusTurnOffPostEffect) {
			postEffectWasOnWhenEnteringDarkenModePlusTurnOffPostEffect = game.postEffectOn() != 0;
			game.postEffectOn() = false;
		} else if (postEffectWasOnWhenEnteringDarkenModePlusTurnOffPostEffect) {
			game.postEffectOn() = true;
		}
	}
}
