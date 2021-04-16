#pragma once
#include "stdafx.h"

namespace RHI
{

	struct FRHIResource
	{
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

	//struct FIndividual : public FRHIResource //TODO: FIndividual do not store mesh and res data, but the obj mesh index, and pos
	//{
	//	shared_ptr<FMesh> Mesh;
	//	shared_ptr<FMeshRes> MeshRes;
	//};

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
}
