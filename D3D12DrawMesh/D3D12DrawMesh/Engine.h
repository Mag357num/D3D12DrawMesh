//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSampleHelper.h"
#include "SimpleCamera.h"
#include "DynamicRHI.h"
#include "StepTimer.h"
#include "FScene.h"
#include "AssetManager.h"

class FEngine;
extern FEngine* GEngine;

class FEngine
{
public:
    FEngine(uint32 width, uint32 height, std::wstring name);
    virtual ~FEngine();

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();

    // Samples override the event handlers to handle specific messages.
	void OnKeyDown(unsigned char Key);
	void OnKeyUp(unsigned char Key);
    void OnMouseMove(uint32 x, uint32 y);
	void OnRightButtonDown(uint32 x, uint32 y);
	void OnRightButtonUp();

    // Accessors.
    uint32 GetWidth() const { return ResoWidth; }
    uint32 GetHeight() const { return ResoHeight; }
    const wchar_t* GetTitle() const { return Title.c_str(); }
    StepTimer& GetTimer() { return Timer; }

protected:
    // Viewport dimensions.
    uint32 ResoWidth;
    uint32 ResoHeight;
    float AspectRatio;

    // Adapter info.
    bool IsUseWarpDevice;

private:
    // Root assets path.
    std::wstring AssetsPath;

    // Window title.
    std::wstring Title;

    // timer
    StepTimer Timer;

    // scene
	shared_ptr <FScene> CurrentScene;

    // manager
	shared_ptr<FAssetManager> AssetManager;
};
