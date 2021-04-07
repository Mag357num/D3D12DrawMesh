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
#include "Renderer.h"
#include "SimpleCamera.h"

class DXSample
{
public:
    DXSample(UINT width, UINT height, std::wstring name);
    ~DXSample();

    // Samples override the event handlers to handle specific messages.
    void OnKeyDown(UINT8 key, Camera Cam)
    {
        Cam.OnKeyUp(key);
    }

    void OnKeyUp(UINT8 key, Camera Cam)
    {
        Cam.OnKeyUp(key);
    }

    // Accessors.
    UINT GetWidth() const           { return ResoWidth; }
    UINT GetHeight() const          { return ResoHeight; }
    const WCHAR* GetTitle() const   { return m_title.c_str(); }

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);

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
};
