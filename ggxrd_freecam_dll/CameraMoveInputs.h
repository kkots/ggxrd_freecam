#pragma once

struct CameraMoveInputs {
	float lookRightSpeed = 0.F;
	float lookLeftSpeed = 0.F;
	float lookUpSpeed = 0.F;
	float lookDownSpeed = 0.F;
	float rollRightSpeed = 0.F;
	float rollLeftSpeed = 0.F;
	float cameraMoveForwardSpeed = 0.F;
	float cameraMoveBackwardsSpeed = 0.F;
	float cameraMoveRightSpeed = 0.F;
	float cameraMoveLeftSpeed = 0.F;
	float cameraMoveUpSpeed = 0.F;
	float cameraMoveDownSpeed = 0.F;
	float increaseFovSpeed = 0.F;
	float decreaseFovSpeed = 0.F;
	bool needResetRollFov = false;
};
