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
		virtual void Init() {};
	};

	struct FShader : public FRHIResource
	{
		virtual void Init() {};
		std::wstring FileName;
	};

	struct FMesh : public FRHIResource
	{
		virtual void Init() {};

		FMesh() : PVertData(nullptr), PIndtData(nullptr), VertexBufferSize(0), VertexStride(0), IndexBufferSize(0), IndexNum(0) {}
		~FMesh() { if (PVertData != nullptr) { free(PVertData); } if (PIndtData != nullptr) { free(PIndtData); } }

		void* PVertData;
		void* PIndtData;
		int VertexBufferSize;
		int VertexStride;
		int IndexBufferSize;
		int IndexNum;
	};

	struct FTransform
	{
		FMatrix Translation = glm::mat4(1.0f); // TODO: changes to FVector
		FVector Rotator;
		FVector Scale;
	};

	struct FMeshRes : public FRHIResource
	{
		virtual void init() {};

		FTransform Transform;
		shared_ptr<FCB> CB;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
	};

	struct FIndividual : public FRHIResource //TODO: Actor do not store mesh and res data, but the obj mesh index, and pos
	{
		shared_ptr<FMesh> Mesh;
		shared_ptr<FMeshRes> MeshRes;
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
}
