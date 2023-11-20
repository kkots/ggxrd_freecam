#pragma once
#include <d3d9.h>
#include <mutex>
#include "CameraMoveInputs.h"
#include "Vector3.h"

using updateDarken_t = void(__thiscall*)(char* thisArg);
using updateCamera_t = void(__thiscall*)(char* thisArg, char** param1, char* param2);

class Camera
{
public:
	bool onDllMain();
	void updateDarkenHook(char* thisArg);
	void updateCameraHook(char* thisArg, char** param1, char* param2);
	void updateCameraManually(CameraMoveInputs& inputs);
	bool isFreecamMode = false;
	bool darkenMode = false;
private:
	struct CameraValues {
		float x = 0.F;
		float y = 0.F;
		float z = 0.F;
		int pitch = 0;
		int yaw = 0;
		int roll = 0;
		float fov = 0.F;
	};
	class HookHelp {
		friend class Camera;
		void updateDarkenHook();
		void updateCameraHook(char** param1, char* param2);
	};
	updateDarken_t orig_updateDarken = nullptr;
	std::mutex orig_updateDarkenMutex;
	updateCamera_t orig_updateCamera = nullptr;
	std::mutex orig_updateCameraMutex;
	unsigned int darkenValue1Offset = 0;
	unsigned int cameraOffset = 0;
	void anglesToDirections(int pitch, int yaw, int roll, Vector3& forward, Vector3& up, Vector3& left, Vector3& yawAxis) const;
	void capAngle(int& angle);
};

extern Camera camera;
