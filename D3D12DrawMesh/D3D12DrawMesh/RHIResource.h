#pragma once
#include "stdafx.h"
#include "Light.h"

namespace RHI
{
	struct FRHIResource
	{
		virtual ~FRHIResource() = default;
	};

	struct FRasterizer : public FRHIResource
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
	};

	struct FShader : public FRHIResource
	{
		std::wstring FileName;
	};

	struct FMesh : public FRHIResource
	{
		FMesh() = default;
		uint32 IndexNum;
	};

	struct FMeshRes : public FRHIResource
	{
		shared_ptr<FCB> BaseCB;
		shared_ptr<FCB> ShadowCB;
		shared_ptr<FRasterizer> BaseRas;
		shared_ptr<FRasterizer> ShadowRas;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
	};

	struct FRACreater
	{
		virtual void InitPsoInitializer() = 0;
	};

	struct FTexture : public FRHIResource
	{
		virtual ~FTexture() = default;
	};

	struct FTexture2D : public FTexture
	{
	};

	struct FSampler
	{
		virtual ~FSampler() = default;
	};

	struct FRenderTarget
	{
		virtual ~FRenderTarget() = default;
	};

	struct FBlinnPhongCB
	{
		FMatrix World;
		FMatrix ViewProj;
		FVector4 CamEye;
		FDirectionLight Light;
	};

	struct FShadowMapCB // BlinnPhong
	{
		FMatrix World;
		FMatrix CamViewProj;
		FMatrix ShadowTransForm;
		FVector4 CamEye;
		FDirectionLight Light;
		BOOL IsShadowMap; // TODO: BOOL is win dependent?
		float padding[3];
	};

	enum class FTextureType
	{
		SHADOW_MAP = 0,
		DEPTH_STENCIL_MAP = 1,
		SHADER_RESOURCE = 2,
	};

	enum class FViewType
	{
		Dsv = 0,
		Srv = 1,
	};

	enum class FSamplerType
	{
		CLAMP = 0,
		WARP = 1,
		MIRROR = 2,
	};

	enum class FRtType
	{
		EMPTY = 0,
		SWAPCHAIN = 1,
	};

	enum class FPassType
	{
		SHADOW_PASS = 0,
		BASE_PASS = 1,
	};

	enum class FRESOURCE_STATES
	{
		D3D12_RESOURCE_STATE_COMMON = 0,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		D3D12_RESOURCE_STATE_INDEX_BUFFER = 0x2,
		D3D12_RESOURCE_STATE_RENDER_TARGET = 0x4,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		D3D12_RESOURCE_STATE_DEPTH_WRITE = 0x10,
		D3D12_RESOURCE_STATE_DEPTH_READ = 0x20,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		D3D12_RESOURCE_STATE_STREAM_OUT = 0x100,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		D3D12_RESOURCE_STATE_COPY_DEST = 0x400,
		D3D12_RESOURCE_STATE_COPY_SOURCE = 0x800,
		D3D12_RESOURCE_STATE_RESOLVE_DEST = 0x1000,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
		D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE = 0x1000000,
		D3D12_RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		D3D12_RESOURCE_STATE_PRESENT = 0,
		D3D12_RESOURCE_STATE_PREDICATION = 0x200,
		D3D12_RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
		D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
		D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
		D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000,
		D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ = 0x200000,
		D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE = 0x800000
	};
}
