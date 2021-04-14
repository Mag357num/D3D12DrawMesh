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

	void UpdateMainCamera();

    // Samples override the event handlers to handle specific messages.
	void OnKeyDown(unsigned char Key);
    void OnKeyUp(unsigned char Key);

    // Accessors.
    uint32 GetWidth() const { return ResoWidth; }
    uint32 GetHeight() const { return ResoHeight; }
    const wchar_t* GetTitle() const { return m_title.c_str(); }
    StepTimer& GetTimer() { return Timer; }

protected:
    // Viewport dimensions.
    uint32 ResoWidth;
    uint32 ResoHeight;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

    // timer
    StepTimer Timer;

    // scene
	shared_ptr <FScene> CurrentScene;
};
