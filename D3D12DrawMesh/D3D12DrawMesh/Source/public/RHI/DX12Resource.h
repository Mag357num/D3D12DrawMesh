#pragma once

#include "stdafx.h"
#include "RHIResource.h"

#include <d2d1_3.h>
#include <dwrite.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <pix.h>
using namespace DirectX; // TODO: namespace DirectX is platform dependent
using Microsoft::WRL::ComPtr;

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
	};

	struct FDX12Mesh_new : public FMesh_new
	{
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;
	};

	struct FDX12Mesh_deprecated : public FMesh_deprecated
	{
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;
	};

	struct FDX12RootSignatrue : public FRootSignatrue
	{
		ComPtr<ID3D12RootSignature> RootSignature;
	};
}