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

#include "stdafx.h"
#include "DrawMesh.h"
#include "SimpleCamera.h"
#include <fstream>
#include "DynamicRHI.h"

using namespace RHI;
using RHI::GDynamicRHI;

FDrawMesh::FDrawMesh(UINT Width, UINT Height, std::wstring Name) :
	DXSample(Width, Height, Name)
{
}

void FDrawMesh::ReadCameraBinary(const string & BinFileName, XMFLOAT3 & Location, XMFLOAT3 & Dir, float & Fov, float & Aspect, XMFLOAT4 & Rotator)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}
	Fin.read((char*)&Location, sizeof(float) * 3);
	Fin.read((char*)&Dir, sizeof(float) * 3);
	Fin.read((char*)&Fov, sizeof(float));
	Fin.read((char*)&Aspect, sizeof(float));
	Fin.read((char*)&Rotator, sizeof(float) * 4);
	//MainCamera.Init({ 500, 0, 0 }, {0, 0, 1}, { -1, 0, 0});
	Fin.close();
}

void FDrawMesh::OnInit()
{
	//read cam binary
	XMFLOAT3 Location;
	XMFLOAT3 Dir;
	float Fov;
	float Aspect;
	XMFLOAT4 Rotator;
	ReadCameraBinary("SingleCameraBinary_.dat", Location, Dir, Fov, Aspect, Rotator);

	//FDynamicRHI::CreateRHI();
	//GDynamicRHI->RHIInit(m_useWarpDevice, BufferFrameCount, ResoWidth, ResoHeight);
}


void FDrawMesh::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	//WaitForPreviousFrame(); // TODO: Uncomment this

	//CloseHandle(FenceEvent);
}

void FDrawMesh::OnKeyDown(UINT8 Key)
{
	//MainCamera.OnKeyDown(Key);
}

void FDrawMesh::OnKeyUp(UINT8 Key)
{
	//MainCamera.OnKeyUp(Key);
}