#pragma once
#include <d3d9.h>
#include <vector>
#include <condition_variable>
#include <mutex>
#include "Controls.h"
#include "InputData.h"

using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
using Present_t = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);
using Reset_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS* pPresentationParameters);
using BBScr_createParticleWithArg_t = void(__thiscall*)(void* pawn, const char* animName, unsigned int posType);
using BBScr_linkParticle_t = void(__thiscall*)(void* pawn, const char* name);

HRESULT __stdcall hook_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT __stdcall hook_EndScene(IDirect3DDevice9* device);
HRESULT __stdcall hook_Present(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);

class EndScene
{
public:
	bool onDllMain();
	void onDllDetach();
	HRESULT presentHook(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);
	void endSceneHook(IDirect3DDevice9* device);
	void resetHook();
	bool endSceneOnlyProcessKeys();
	void finishProcessingKeys();
	void setPresentFlag();
	bool consumePresentFlag();
	void clearContinuousScreenshotMode();
	void onGifModeBlackBackgroundChanged();
	EndScene_t orig_EndScene = nullptr;
	std::mutex orig_EndSceneMutex;
	Present_t orig_Present = nullptr;
	std::mutex orig_PresentMutex;
	Reset_t orig_Reset = nullptr;
	std::mutex orig_ResetMutex;
	bool modDisabled = false;
	
	BBScr_createParticleWithArg_t orig_BBScr_createParticleWithArg = nullptr;
	std::mutex orig_BBScr_createParticleWithArgMutex;
	bool orig_BBScr_createParticleWithArgMutexLocked = false;
	BBScr_linkParticle_t orig_BBScr_linkParticle = nullptr;
	std::mutex orig_BBScr_linkParticleMutex;
	bool orig_BBScr_linkParticleMutexLocked = false;
	BBScr_linkParticle_t orig_BBScr_linkParticleWithArg2 = nullptr;
	std::mutex orig_BBScr_linkParticleWithArg2Mutex;
	bool orig_BBScr_linkParticleWithArg2MutexLocked = false;
	
	bool allowCreateParticlesBase = true;
	bool allowCreateParticlesToggle = false;
	inline bool allowCreateParticles() const {
		return allowCreateParticlesBase ^ allowCreateParticlesToggle;
	}
private:
	class HookHelp {
		friend class EndScene;
		void BBScr_createParticleWithArgHook(const char* animName, unsigned int posType);
		void BBScr_linkParticleHook(const char* name);
		void BBScr_linkParticleWithArg2Hook(const char* name);
	};
	void BBScr_createParticleWithArgHook(void* pawn, const char* animName, unsigned int posType);
	void BBScr_linkParticleHook(void* pawn, const char* name);
	void BBScr_linkParticleWithArg2Hook(void* pawn, const char* name);

	struct HiddenEntity {
		char* ent = nullptr;
		int scaleX = 0;
		int scaleY = 0;
		int scaleZ = 0;
		int scaleDefault = 0;
		bool wasFoundOnThisFrame = false;
	};
	void tempPressKey(int code);
	void releaseTempKeys();
	void noGravGifMode();
	std::vector<HiddenEntity>::iterator findHiddenEntity(char* ent);
	
	// The EndScene function is actually being called twice: once by GuiltyGear and one more time by the Steam overlay.
	// However, Present is only called once each frame. So we use the Present function to determine if the next EndScene
	// call should draw the boxes.
	bool presentCalled = true;
	InputData inputData;
	bool magnetCursorMode = false;

	std::vector<int> tempPressedKeys;
	int hasBeenMovingFor = 0;
	unsigned int allowNextFrameBeenHeldFor = 0;
	unsigned int allowNextFrameCounter = 0;
	bool freezeGame = false;

	bool needToTakeScreenshot = false;
	bool continuousScreenshotMode = false;
	bool needContinuouslyTakeScreens = false;
	unsigned int p1PreviousTimeOfTakingScreen = ~0;
	unsigned int p2PreviousTimeOfTakingScreen = ~0;

	bool hideOpponent = false;
	std::vector<HiddenEntity> hiddenEntities;
	bool postEffectWasOnWhenEnteringDarkenModePlusTurnOffPostEffect = false;
	bool isInDarkenModePlusTurnOffPostEffect = false;
};

extern EndScene endScene;
