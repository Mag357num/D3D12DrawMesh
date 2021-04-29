#include "DynamicRHI.h"
#include "DXSampleHelper.h"
#include "DX12DynamicRHI.h"

using namespace RHI;

// Globals.
shared_ptr<FDynamicRHI> RHI::GDynamicRHI = nullptr;

void FDynamicRHI::CreateRHI()
{
#if defined (_WINDOWS)
	RHI::GDynamicRHI = make_shared<FDX12DynamicRHI>();
#else
#error("No avaible RHI.")
#endif
}