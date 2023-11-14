#include "pch.h"
#include "Camera.h"
#include "Game.h"
#include "memoryFunctions.h"
#include "Detouring.h"
#include "pi.h"
#include "logging.h"

Camera camera;

const unsigned int cameraCoordsStart = 0x3C0;
const float coordCoefficient = 0.42960999705207F;

bool Camera::onDllMain() {
	bool error = false;

	if (aswEngine) {
		char cameraOffsetSig[] = "\x8b\x4c\x24\x18\x83\xc4\x08\x0b\x4c\x24\x14\x74\x1e\x8b\x15\x00\x00\x00\x00\x8b\x8a\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x85\xc0\x75\x09\x55\xe8\x00\x00\x00\x00\x83\xc4\x04";
		char cameraOffsetSigMask[] = "xxxxxxxxxxxxxxx????xx????x????xxxxxx????xxx";

		substituteWildcard(cameraOffsetSigMask, cameraOffsetSig, (char*)&aswEngine, 4, 0);

		cameraOffset = (unsigned int)sigscanOffset(
			"GuiltyGearXrd.exe",
			cameraOffsetSig,
			cameraOffsetSigMask,
			{ 0x15, 0 },
			&error, "cameraOffset");

	}

	orig_updateCamera = (updateCamera_t)sigscanOffset(
		"GuiltyGearXrd.exe",
		"\x55\x8b\xec\x83\xe4\xf0\x83\xec\x00\x53\x56\x8b\xf1\x83\xbe\x00\x00\x00\x00\x00\x57\x8b\x7d\x08\x74\x17\x8d\x86\x00\x00\x00\x00\x3b\xf8\x75\x0d\xf6\x86\x00\x00\x00\x00\x01\x0f\x85\x18\x01\x00\x00\xf6\x86\x00\x00\x00\x00\x03\x0f\x84\x0b\x01\x00\x00\x8b\x0f\xd9\x86\x00\x00\x00\x00",
		"xxxxxxxx?xxxxxx????xxxxxxxxx????xxxxxx????xxxxxxxxx????xxxxxxxxxxx????",
		&error, "updateCamera");

	if (!error) {
		void(HookHelp::*updateCameraHookPtr)(char**, char*) = &HookHelp::updateCameraHook;
		detouring.attach(&(PVOID&)(orig_updateCamera),
			(PVOID&)updateCameraHookPtr,
			&orig_updateCameraMutex,
			"updateCamera");
	}

	return !error;
}

void Camera::HookHelp::updateCameraHook(char** param1, char* param2) {
	HookTracker hookTracker;
	camera.updateCameraHook((char*)this, param1, param2);
}

float radius = 100;
int angle = 0;

void Camera::updateCameraHook(char* thisArg, char** param1, char* param2) {
	if (!isFreecamMode) {
		std::unique_lock<std::mutex> guard(orig_updateCameraMutex);
		orig_updateCamera(thisArg, param1, param2);
	}
}

void Camera::updateCameraManually(CameraMoveInputs& inputs) {
	if (!aswEngine || !*aswEngine) return;
	char* camPtr = *(char**)(*aswEngine + cameraOffset);
	if (!camPtr) return;
	CameraValues* cam = (CameraValues*)(camPtr + 0x384);
	
	float rollRadians = (float)cam->roll / 32768.F * PI;
	float cosRoll = cos(rollRadians);
	float sinRoll = sin(rollRadians);

	float horizDeltaAngle = inputs.lookRightSpeed - inputs.lookLeftSpeed;
	float horizDeltaAngleRadians = horizDeltaAngle / 32768.F * PI;
	float sinHorizDeltaAngle = sin(horizDeltaAngleRadians);
	float cosHorizDeltaAngle = cos(horizDeltaAngleRadians);
	float deltaYawRadians = atan2f(cosRoll * sinHorizDeltaAngle, cosHorizDeltaAngle);
	float deltaPitchRadians = asinf(-sinRoll * sinHorizDeltaAngle);
	cam->yaw += (int)(deltaYawRadians / PI * 32768.F);
	cam->pitch += (int)(deltaPitchRadians / PI * 32768.F);

	float vertDeltaAngle = inputs.lookUpSpeed - inputs.lookDownSpeed;
	float vertDeltaAngleRadians = vertDeltaAngle / 32768.F * PI;
	float sinVertDeltaAngle = sin(vertDeltaAngleRadians);
	float cosVertDeltaAngle = cos(vertDeltaAngleRadians);
	deltaYawRadians = atan2f(sinRoll * sinVertDeltaAngle, cosVertDeltaAngle);
	deltaPitchRadians = asinf(cosRoll * sinVertDeltaAngle);
	cam->yaw += (int)(deltaYawRadians / PI * 32768.F);
	cam->pitch += (int)(deltaPitchRadians / PI * 32768.F);

	cam->roll += (int)inputs.rollRightSpeed - (int)inputs.rollLeftSpeed;

	cam->fov += (int)inputs.increaseFovSpeed - (int)inputs.decreaseFovSpeed;
	if (cam->fov > 179) cam->fov = 179;
	if (cam->fov < 1) cam->fov = 1;

	if (inputs.needResetRollFov) {
		cam->roll = 0;
		cam->fov = 54.F;
	}

	capAngle(cam->pitch);
	capAngle(cam->yaw);
	capAngle(cam->roll);

	Vector3 forward{ 0.F, 0.F , 0.F };
	Vector3 up{ 0.F, 0.F , 0.F };
	Vector3 left{ 0.F, 0.F , 0.F };
	Vector3 yawAxis{ 0.F, 0.F , 0.F };
	anglesToDirections(cam->pitch, cam->yaw, cam->roll, forward, up, left, yawAxis);

	float forwardSpeed = inputs.cameraMoveForwardSpeed - inputs.cameraMoveBackwardsSpeed;
	float leftSpeed = inputs.cameraMoveLeftSpeed - inputs.cameraMoveRightSpeed;
	float upSpeed = inputs.cameraMoveUpSpeed - inputs.cameraMoveDownSpeed;
	cam->x += forward.x * forwardSpeed + left.x * leftSpeed + yawAxis.x * upSpeed;
	cam->y += forward.y * forwardSpeed + left.y * leftSpeed + yawAxis.y * upSpeed;
	cam->z += forward.z * forwardSpeed + left.z * leftSpeed + yawAxis.z * upSpeed;

	CameraValues* pendingCam = *(CameraValues**)(camPtr + cameraCoordsStart) + 0x54;
	*pendingCam = *cam;

	pendingCam = (CameraValues*)(camPtr + 0x3C8);
	*pendingCam = *cam;
}

void Camera::anglesToDirections(int pitch, int yaw, int roll, Vector3& forward, Vector3& up, Vector3& left, Vector3& yawAxis) const {
	float pitchFloat = (float)pitch / 32768.F * PI;
	float yawFloat = (float)yaw / 32768.F * PI;
	float rollFloat = (float)roll / 32768.F * PI;

	float sinPitch = sin(pitchFloat);
	float cosPitch = cos(pitchFloat);

	float sinYaw = sin(yawFloat);
	float cosYaw = cos(yawFloat);

	float sinRoll = sin(rollFloat);
	float cosRoll = cos(rollFloat);

	forward.x = cosYaw * cosPitch;
	forward.y = sinYaw * cosPitch;
	forward.z = sinPitch;

	up.x = cosYaw * sinPitch * cosRoll - sinYaw * sinRoll;
	up.y = cosYaw * sinRoll - sinYaw * sinPitch * cosRoll;
	up.z = cosPitch * cosRoll;

	left.x = sinYaw * cosRoll - sinPitch * cosYaw * sinRoll;
	left.y = sinPitch * sinYaw * sinRoll - cosYaw * cosRoll;
	left.z = cosPitch * sinRoll;

	yawAxis.x = sinRoll * sinYaw;
	yawAxis.y = sinRoll * cosYaw;
	yawAxis.z = cosRoll;

}

void Camera::capAngle(int& angle) {
	
	angle = angle % 65536;
	if (angle > 32768) {
		angle -= 65536;
	} else if (angle < -32768) {
		angle += 65536;
	}
}
