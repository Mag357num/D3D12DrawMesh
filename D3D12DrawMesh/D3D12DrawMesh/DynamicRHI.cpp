#include "DynamicRHI.h"
#include "DXSampleHelper.h"
#include "Renderer.h"
#include "DX12DynamicRHI.h"

using namespace RHI;

// Globals.
FDynamicRHI* RHI::GDynamicRHI = nullptr;


void FDynamicRHI::CreateRHI()
{
#if defined (_WINDOWS)
	RHI::GDynamicRHI = new RHI::FDX12DynamicRHI();
#else
#error("No avaible RHI.")
#endif
}