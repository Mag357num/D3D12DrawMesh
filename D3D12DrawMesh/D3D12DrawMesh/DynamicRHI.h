#pragma once
#include "stdafx.h"
#include "DXSample.h"

namespace RHI
{
	class DynamicRHI
	{
	public:
		DynamicRHI() = default;
		~DynamicRHI() = default;

		virtual void CreateDevice() = 0;
		virtual void CreateSwapChain() = 0;
		virtual void GetDevice() = 0;

		virtual void CreateCommandQueue() = 0;
		virtual void ExecuteCommand() = 0;

		virtual void UpdateVertexBuffer() = 0;
		virtual void UpdateIndexBuffer() = 0;
	};

	class D3D12DynamicRHI : public DynamicRHI
	{
	public:
		D3D12DynamicRHI() = default;
		~D3D12DynamicRHI() = default;

	public:
		ComPtr<ID3D12Device>				Device;
		ComPtr<IDXGIFactory4>				Factory;
		ComPtr<IDXGISwapChain3>				SwapChain;
		ComPtr<ID3D12GraphicsCommandList>	MainCommandList;
		ComPtr<ID3D12CommandQueue>			CommandQueue;
		ComPtr<ID3D12CommandAllocator>		MainCommandAllocator;
	}


}