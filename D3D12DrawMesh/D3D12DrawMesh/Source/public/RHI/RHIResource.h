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

	struct FHandle : public FRHIResource
	{
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
		shared_ptr<FHandle> CBHandle;
	};

	struct FShader : public FRHIResource
	{
		std::wstring FileName;
	};

	enum class FFormat;
	struct FInputElement // same with D3D12_INPUT_ELEMENT_DESC
	{
		string SemanticName;
		uint32 SemanticIndex;
		FFormat Format;
		uint32 InputSlot;
		uint32 AlignedByteOffset;
		uint32 InputSlotClass;
		uint32 InstanceDataStepRate;
	};

	struct FVertexInputLayer
	{
		vector<FInputElement> Elements;
	};

	struct FRootSignatrue : public FRHIResource {};

	struct FRenderResource
	{
		shared_ptr<FPipelineState> PSO;
		shared_ptr<FRootSignatrue> Sig;
		shared_ptr<FShader> VS;
		shared_ptr<FShader> PS;
		vector<shared_ptr<FCB>> CBs; // is a vector beacuse multi buffering
	};

	struct FGeometry : public FRHIResource
	{
		uint32 IndexNum;
	};

	enum class FRESOURCE_STATES;
	struct FTexture : public FRHIResource
	{
		virtual ~FTexture() = default;

		FRESOURCE_STATES TexState;
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
		shared_ptr<FHandle> SamplerHandle;
	};

	enum class FRangeType;
	enum class FShaderVisibility;
	struct FShaderInputElement
	{
		FRangeType RangeType;
		uint32 NumDescriptors;
		uint32 BaseShaderRegister;
		FShaderVisibility Visibility;
	};

	struct FShaderInputLayer
	{
		vector<FShaderInputElement> Elements;
	};

	enum class FRangeType
	{
		DESCRIPTOR_RANGE_TYPE_SRV = 0,
		DESCRIPTOR_RANGE_TYPE_UAV = (DESCRIPTOR_RANGE_TYPE_SRV + 1),
		DESCRIPTOR_RANGE_TYPE_CBV = (DESCRIPTOR_RANGE_TYPE_UAV + 1),
		DESCRIPTOR_RANGE_TYPE_SAMPLER = (DESCRIPTOR_RANGE_TYPE_CBV + 1)
	};

	enum class FShaderVisibility
	{
		SHADER_VISIBILITY_ALL = 0,
		SHADER_VISIBILITY_VERTEX = 1,
		SHADER_VISIBILITY_HULL = 2,
		SHADER_VISIBILITY_DOMAIN = 3,
		SHADER_VISIBILITY_GEOMETRY = 4,
		SHADER_VISIBILITY_PIXEL = 5,
		SHADER_VISIBILITY_AMPLIFICATION = 6,
		SHADER_VISIBILITY_MESH = 7
	};

	enum class FTextureType
	{
		SHADOW_MAP_TT = 0,
		DEPTH_STENCIL_MAP_TT = 1,
		SCENE_COLOR_TT = 2,
		RENDER_TARGET_TEXTURE_TT = 3,
		ORDINARY_SHADER_RESOURCE_TT = 4, // for texture that only act as srv but not for ds, rt etc... should read from file
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

	enum class FRESOURCE_STATES // enum values are same with dx
	{
		RESOURCE_STATE_COMMON = 0,
		RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		RESOURCE_STATE_INDEX_BUFFER = 0x2,
		RESOURCE_STATE_RENDER_TARGET = 0x4,
		RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		RESOURCE_STATE_DEPTH_WRITE = 0x10,
		RESOURCE_STATE_DEPTH_READ = 0x20,
		RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		RESOURCE_STATE_STREAM_OUT = 0x100,
		RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		RESOURCE_STATE_COPY_DEST = 0x400,
		RESOURCE_STATE_COPY_SOURCE = 0x800,
		RESOURCE_STATE_RESOLVE_DEST = 0x1000,
		RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
		RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
		RESOURCE_STATE_SHADING_RATE_SOURCE = 0x1000000,
		RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		RESOURCE_STATE_PRESENT = 0,
		RESOURCE_STATE_PREDICATION = 0x200,
		RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
		RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
		RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
		RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000,
		RESOURCE_STATE_VIDEO_ENCODE_READ = 0x200000,
		RESOURCE_STATE_VIDEO_ENCODE_WRITE = 0x800000
	};

	enum class FFormat // enum values are same with dx
	{
		FORMAT_UNKNOWN = 0,
		FORMAT_R32G32B32A32_TYPELESS = 1,
		FORMAT_R32G32B32A32_FLOAT = 2,
		FORMAT_R32G32B32A32_UINT = 3,
		FORMAT_R32G32B32A32_SINT = 4,
		FORMAT_R32G32B32_TYPELESS = 5,
		FORMAT_R32G32B32_FLOAT = 6,
		FORMAT_R32G32B32_UINT = 7,
		FORMAT_R32G32B32_SINT = 8,
		FORMAT_R16G16B16A16_TYPELESS = 9,
		FORMAT_R16G16B16A16_FLOAT = 10,
		FORMAT_R16G16B16A16_UNORM = 11,
		FORMAT_R16G16B16A16_UINT = 12,
		FORMAT_R16G16B16A16_SNORM = 13,
		FORMAT_R16G16B16A16_SINT = 14,
		FORMAT_R32G32_TYPELESS = 15,
		FORMAT_R32G32_FLOAT = 16,
		FORMAT_R32G32_UINT = 17,
		FORMAT_R32G32_SINT = 18,
		FORMAT_R32G8X24_TYPELESS = 19,
		FORMAT_D32_FLOAT_S8X24_UINT = 20,
		FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		FORMAT_R10G10B10A2_TYPELESS = 23,
		FORMAT_R10G10B10A2_UNORM = 24,
		FORMAT_R10G10B10A2_UINT = 25,
		FORMAT_R11G11B10_FLOAT = 26,
		FORMAT_R8G8B8A8_TYPELESS = 27,
		FORMAT_R8G8B8A8_UNORM = 28,
		FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		FORMAT_R8G8B8A8_UINT = 30,
		FORMAT_R8G8B8A8_SNORM = 31,
		FORMAT_R8G8B8A8_SINT = 32,
		FORMAT_R16G16_TYPELESS = 33,
		FORMAT_R16G16_FLOAT = 34,
		FORMAT_R16G16_UNORM = 35,
		FORMAT_R16G16_UINT = 36,
		FORMAT_R16G16_SNORM = 37,
		FORMAT_R16G16_SINT = 38,
		FORMAT_R32_TYPELESS = 39,
		FORMAT_D32_FLOAT = 40,
		FORMAT_R32_FLOAT = 41,
		FORMAT_R32_UINT = 42,
		FORMAT_R32_SINT = 43,
		FORMAT_R24G8_TYPELESS = 44,
		FORMAT_D24_UNORM_S8_UINT = 45,
		FORMAT_R24_UNORM_X8_TYPELESS = 46,
		FORMAT_X24_TYPELESS_G8_UINT = 47,
		FORMAT_R8G8_TYPELESS = 48,
		FORMAT_R8G8_UNORM = 49,
		FORMAT_R8G8_UINT = 50,
		FORMAT_R8G8_SNORM = 51,
		FORMAT_R8G8_SINT = 52,
		FORMAT_R16_TYPELESS = 53,
		FORMAT_R16_FLOAT = 54,
		FORMAT_D16_UNORM = 55,
		FORMAT_R16_UNORM = 56,
		FORMAT_R16_UINT = 57,
		FORMAT_R16_SNORM = 58,
		FORMAT_R16_SINT = 59,
		FORMAT_R8_TYPELESS = 60,
		FORMAT_R8_UNORM = 61,
		FORMAT_R8_UINT = 62,
		FORMAT_R8_SNORM = 63,
		FORMAT_R8_SINT = 64,
		FORMAT_A8_UNORM = 65,
		FORMAT_R1_UNORM = 66,
		FORMAT_R9G9B9E5_SHAREDEXP = 67,
		FORMAT_R8G8_B8G8_UNORM = 68,
		FORMAT_G8R8_G8B8_UNORM = 69,
		FORMAT_BC1_TYPELESS = 70,
		FORMAT_BC1_UNORM = 71,
		FORMAT_BC1_UNORM_SRGB = 72,
		FORMAT_BC2_TYPELESS = 73,
		FORMAT_BC2_UNORM = 74,
		FORMAT_BC2_UNORM_SRGB = 75,
		FORMAT_BC3_TYPELESS = 76,
		FORMAT_BC3_UNORM = 77,
		FORMAT_BC3_UNORM_SRGB = 78,
		FORMAT_BC4_TYPELESS = 79,
		FORMAT_BC4_UNORM = 80,
		FORMAT_BC4_SNORM = 81,
		FORMAT_BC5_TYPELESS = 82,
		FORMAT_BC5_UNORM = 83,
		FORMAT_BC5_SNORM = 84,
		FORMAT_B5G6R5_UNORM = 85,
		FORMAT_B5G5R5A1_UNORM = 86,
		FORMAT_B8G8R8A8_UNORM = 87,
		FORMAT_B8G8R8X8_UNORM = 88,
		FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		FORMAT_B8G8R8A8_TYPELESS = 90,
		FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		FORMAT_B8G8R8X8_TYPELESS = 92,
		FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		FORMAT_BC6H_TYPELESS = 94,
		FORMAT_BC6H_UF16 = 95,
		FORMAT_BC6H_SF16 = 96,
		FORMAT_BC7_TYPELESS = 97,
		FORMAT_BC7_UNORM = 98,
		FORMAT_BC7_UNORM_SRGB = 99,
		FORMAT_AYUV = 100,
		FORMAT_Y410 = 101,
		FORMAT_Y416 = 102,
		FORMAT_NV12 = 103,
		FORMAT_P010 = 104,
		FORMAT_P016 = 105,
		FORMAT_420_OPAQUE = 106,
		FORMAT_YUY2 = 107,
		FORMAT_Y210 = 108,
		FORMAT_Y216 = 109,
		FORMAT_NV11 = 110,
		FORMAT_AI44 = 111,
		FORMAT_IA44 = 112,
		FORMAT_P8 = 113,
		FORMAT_A8P8 = 114,
		FORMAT_B4G4R4A4_UNORM = 115,

		FORMAT_P208 = 130,
		FORMAT_V208 = 131,
		FORMAT_V408 = 132,

		FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE = 189,
		FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,

	};
}
