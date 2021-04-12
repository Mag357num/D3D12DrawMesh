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
    FEngine(UINT width, UINT height, std::wstring name);
    virtual ~FEngine();

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();

	XMMATRIX UpdateViewProj();

    // Samples override the event handlers to handle specific messages.
	void OnKeyDown(UINT8 Key);
    void OnKeyUp(UINT8 Key);

    // Accessors.
    UINT GetWidth() const { return ResoWidth; }
    UINT GetHeight() const { return ResoHeight; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }

protected:
    // Viewport dimensions.
    UINT ResoWidth;
    UINT ResoHeight;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

    // cam
	FCamera MainCamera;

    // timer
    StepTimer Timer;

    // scene
	shared_ptr <FScene> CurrentScene;
};
