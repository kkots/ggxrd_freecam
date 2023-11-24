#pragma once
#include <d3d9.h>
#include <atlbase.h>
#include <vector>
#include <mutex>

class Screenshotting
{
public:
	void takeScreenshot(IDirect3DDevice9* device);
	void clear(bool isShuttingDown = false);
private:
	bool getFramebufferData(IDirect3DDevice9* device,
		std::vector<unsigned char>& buffer,
		IDirect3DSurface9* renderTarget = nullptr,
		D3DSURFACE_DESC* renderTargetDescPtr = nullptr,
		unsigned int* widthPtr = nullptr,
		unsigned int* heightPtr = nullptr);
	IDirect3DSurface9* getOffscreenSurface(IDirect3DDevice9* device, D3DSURFACE_DESC* renderTargetDescPtr = nullptr);

	std::mutex offscreenSurfaceMutex;
	bool shuttingDown = false;
	CComPtr<IDirect3DSurface9> offscreenSurface;
	unsigned int offscreenSurfaceWidth = 0;
	unsigned int offscreenSurfaceHeight = 0;
};

extern Screenshotting screenshotting;
