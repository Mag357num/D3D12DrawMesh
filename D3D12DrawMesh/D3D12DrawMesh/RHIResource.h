#pragma once
#include "stdafx.h"

namespace RHI
{
	class FRHIResource
	{
	};

	struct FCBData : public FRHIResource
	{
		FCBData() : BufferData(nullptr), BufferSize(256) {};
		void* BufferData;
		UINT BufferSize;
	};

	struct FShader : public FRHIResource
	{
		virtual void Init() {};
	};

	struct FMesh : public FRHIResource
	{
		virtual void Init() {};

		FMesh() : PVertData(nullptr), PIndtData(nullptr), VertexBufferSize(0), VertexStride(0), IndexBufferSize(0), IndexNum(0) {}
		~FMesh() { free(PVertData); free(PIndtData); } // TODO: free nullptr will clapse

		UINT8* PVertData;
		UINT8* PIndtData;
		int VertexBufferSize;
		int VertexStride;
		int IndexBufferSize;
		int IndexNum;
	};

	struct FMeshRes : public FRHIResource
	{
		FMeshRes() : WorldTrans(XMMatrixTranslation(0.f, 0.f, 0.f)), VS(nullptr), PS(nullptr) {}
		virtual void init() {};

		XMMATRIX WorldTrans;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
	};

	struct FActor : public FRHIResource //TODO: Actor do not store mesh and res data, but the obj mesh index, and pos
	{
		shared_ptr<FMesh> Mesh;
		shared_ptr<FMeshRes> MeshRes;
	};

	struct FPSOInitializer
	{
		virtual void InitPsoInitializer(/*FInputLayout InputLayout, FRHIShader Shader*/) = 0;
	};

	struct FCB : public FRHIResource
	{
	};

	class FTexture : public FRHIResource
	{
	};

	class FTexture2D : public FTexture
	{
	};
}
