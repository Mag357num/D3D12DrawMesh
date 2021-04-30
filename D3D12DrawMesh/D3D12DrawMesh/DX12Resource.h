#pragma once

#include "stdafx.h"
#include "RHIResource.h"

namespace RHI
{
	struct FDX12CpuHandle : FHandle
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE Handle;
	};

	struct FDX12GpuHandle : FHandle
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE Handle;
	};

	struct FDX12Texture : public FTexture
	{
		DXGI_FORMAT SrvFormat;
		DXGI_FORMAT DsvFormat;
		ComPtr<ID3D12Resource> DX12Texture;
	};

	struct FDX12Sampler : public FSampler
	{
		FSamplerType Type;
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplerHandle;
	};

	struct FDX12PipelineState : public FPipelineState
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
}