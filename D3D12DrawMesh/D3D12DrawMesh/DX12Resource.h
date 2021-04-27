#pragma once

#include "stdafx.h"
#include "RHIResource.h"

namespace RHI
{
	struct FDX12Texture : public FTexture
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE DsvCpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGpuHandle;
		ComPtr<ID3D12Resource> DX12Texture;
	};

	struct FDX12Rasterizer : public FRasterizer
	{
		ComPtr<ID3D12PipelineState> PSO;
	};

	struct FDX12Shader : public FShader
	{
		ComPtr<ID3DBlob> Shader;
	};

	struct FDX12CB : public FCB
	{
		ComPtr<ID3D12Resource> CBRes;
		void* UploadBufferVirtualAddress;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandleInHeap;
	};

	struct FDX12MeshRes : public FMeshRes
	{
		//ComPtr<ID3D12PipelineState> BasePSO;
		//ComPtr<ID3D12PipelineState> ShadowPSO;
		ComPtr<ID3D12RootSignature> RootSignature;
	};

	struct FDX12Mesh : public FMesh
	{
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;
	};

	struct FDX12ResourceDesc : public FResourceDesc
	{
		CD3DX12_RESOURCE_DESC ResDesc;
	};

}