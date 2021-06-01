#pragma once
#include "stdafx.h"
#include "Key.h"

class FDeviceEventProcessor
{
private:
	static FDeviceEventProcessor* GDEventProcessor;
	KeysPressed Keys = {};
	bool bIsMouseDown = false;
	FVector2 MouseMove_BottonDown_CurrentPosition = { 0, 0 };
	FVector2 MouseMove_BottonDown_FirstPosition = { 0, 0 };

public:
	static void CreateEventProcessor();
	static void DestroyEventProcessor();
	static FDeviceEventProcessor* Get() { return GDEventProcessor; }

	void Tick();

	void OnKeyDown(const unsigned char& key);
	void OnKeyUp(const unsigned char& key);
	void OnButtonDown(const uint32 & x, const uint32 & y);
	void OnButtonUp();
	void OnMouseMove(const uint32 & x, const uint32 & y);

	const KeysPressed& GetKeys() const { return Keys; }
	const bool& IsMouseDown() const { return bIsMouseDown; }
	FVector2 GetDeltaMouseMove_BottonDown();
};