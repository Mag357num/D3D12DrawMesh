#pragma once
#include "stdafx.h"

namespace RHI
{

	struct FRHIResource
	{
		virtual ~FRHIResource() = default;
	};

	struct FCBData : public FRHIResource
	{
		FCBData() : DataBuffer(nullptr), BufferSize(256) {};
		void* DataBuffer;
		uint32 BufferSize;
	};

	struct FCB : public FRHIResource
	{
		virtual ~FCB() = default;
	};

	struct FShader : public FRHIResource
	{
		virtual ~FShader() = default;
		std::wstring FileName;
	};

	struct FMesh : public FRHIResource
	{
		FMesh() = default;
		virtual ~FMesh() = default;

		uint32 IndexCount;
	};

	struct FMeshRes : public FRHIResource
	{
		virtual ~FMeshRes() = default;

		shared_ptr<FCB> CB;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
	};

	struct FPSOInitializer
	{
		virtual void InitPsoInitializer(/*FInputLayout InputLayout, FRHIShader Shader*/) = 0;
	};

	class FTexture : public FRHIResource
	{
	};

	class FTexture2D : public FTexture
	{
	};

	#pragma pack(1)
	struct FBlinnPhongCB // 168 bytes
	{
		FMatrix WVP;
		FMatrix World;
		FMatrix Rotator;

		FVector4 CamEye;

		FVector4 DirectionLightDir;
		FVector4 DirectionLightColor;
		float DirectionLightIntensity;
	};
	#pragma pack()
}
