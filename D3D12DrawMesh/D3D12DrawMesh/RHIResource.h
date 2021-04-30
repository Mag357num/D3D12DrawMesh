#pragma once
#include "stdafx.h"
#include "Light.h"

namespace RHI
{
	struct FRHIResource
	{
		virtual ~FRHIResource() = default;

		template <typename T>
		inline T* As() { return static_cast<T*>(this); }

		template <typename T>
		inline T* TryAs() { return dynamic_cast<T*>(this); }
	};

	struct FPipelineState : public FRHIResource
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

	struct FRootSignatrue : public FRHIResource
	{
	};

	struct FHandle : public FRHIResource
	{
	};

	enum class FPassType;
	struct FMaterial : public FRHIResource
	{
		FPassType Type;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
		shared_ptr<FCB> CB;
		shared_ptr<RHI::FPipelineState> PSO;
		shared_ptr<FRootSignatrue> Sig;
		vector<shared_ptr<FHandle>> TexHandles;
	};

	struct FMeshRes : public FRHIResource
	{
		shared_ptr<FMaterial> ShadowMat;
		shared_ptr<FMaterial> SceneColorMat;
		shared_ptr<FMaterial> BloomSetupMat;
		shared_ptr<FMaterial> BloomDownMat[4];
		shared_ptr<FMaterial> BloomUpMat[3];
		shared_ptr<FMaterial> SunMergeMat;
		shared_ptr<FMaterial> ToneMappingMat;
	};

	struct FTexture : public FRHIResource
	{
		virtual ~FTexture() = default;
		shared_ptr<FHandle> DsvHandle;
		shared_ptr<FHandle> RtvHandle;
		shared_ptr<FHandle> SrvHandle;
	};

	struct FTexture2D : public FTexture
	{
	};

	struct FSampler : public FRHIResource
	{
		virtual ~FSampler() = default;
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

	struct FBloomSetupCB
	{
		FVector4 BufferSizeAndInvSize;
		float BloomThreshold;
	};

	struct FBloomDownCB
	{
		FVector4 BufferSizeAndInvSize;
		float BloomDownScale;
	};

	struct FBloomUpCB
	{
		FVector4 BufferASizeAndInvSize;
		FVector4 BufferBSizeAndInvSize;
		FVector4 BloomTintA;
		FVector4 BloomTintB;
		FVector2 BloomUpScales;
	};

	struct FSunMergeCB
	{
		FVector4 BloomUpSizeAndInvSize;
		FVector BloomColor;
	};

	enum class FTextureType
	{
		SHADOW_MAP_TT = 0,
		DEPTH_STENCIL_MAP_TT = 1,
		RENDER_TARGET_TT = 2,
		ORDINARY_SHADER_RESOURCE_TT = 3, // for texture that only act as srv but not for ds, rt etc... should read from file
	};

	enum class FResViewType
	{
		DSV_RVT = 0,
		SRV_RVT = 1,
		RTV_RVT = 2,
	};

	enum class FSamplerType
	{
		CLAMP_ST = 0,
		WARP_ST = 1,
		MIRROR_ST = 2,
	};

	enum class FPassType
	{
		SHADOW_PT = 0,
		SCENE_COLOR_PT = 1,
		BLOOM_SETUP_PT = 2,
		BLOOM_DOWN_PT = 3,
		BLOOM_UP_PT = 4,
		SUN_MERGE_PT = 5,
		TONEMAPPING_PT = 6,
	};

	enum class FMeshType_deprecated
	{
		SCENE_MESH_FT = 0,
		PastProcess_MESH_FT = 1,
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
