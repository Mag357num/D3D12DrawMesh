#pragma once

#include "Camera.h"
#include "DynamicRHI.h"
#include "StepTimer.h"
#include "Scene.h"
#include "AssetManager.h"

class FEngine;
extern FEngine* GEngine;

class FEngine
{
public:
	FEngine(uint32 width, uint32 height, std::wstring name);
	virtual ~FEngine();

	void Init(void* WindowHandle);
	void Tick();
	void Render();
	void Destroy();

	void OnKeyDown(unsigned char Key);
	void OnKeyUp(unsigned char Key);
	void OnMouseMove(uint32 x, uint32 y);
	void OnButtonDown(uint32 x, uint32 y);
	void OnButtonUp();

	shared_ptr<TScene> CreateScene() { return make_shared<TScene>(); }

	uint32 GetWidth() const { return ResoWidth; }
	uint32 GetHeight() const { return ResoHeight; }
	const wchar_t* GetTitle() const { return Title.c_str(); }
	StepTimer& GetTimer() { return Timer; }
	void CalculateFrameStats();


protected:
	// Viewport dimensions.
	uint32 ResoWidth;
	uint32 ResoHeight;
	float AspectRatio;

	// Adapter info.
	bool IsUseWarpDevice;

private:
	// window handle
	void* HWindow;

	// Window title.
	std::wstring Title;

	// timer
	StepTimer Timer;

	// scene
	shared_ptr<TScene> CurrentScene;

	// manager
	shared_ptr<FAssetManager> AssetManager;
};
