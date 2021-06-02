#include "DynamicRHI.h"
#include "DX12DynamicRHI.h"

using namespace RHI;

// Globals.
shared_ptr<FDynamicRHI> RHI::GDynamicRHI = nullptr;

void FDynamicRHI::CreateRHI()
{
#if defined (_WINDOWS)
	assert(GDynamicRHI == nullptr);
	GDynamicRHI = make_shared<FDX12DynamicRHI>();
#else
#error("No avaible RHI.")
#endif
}

void RHI::FDynamicRHI::DestroyRHI()
{
	assert(GDynamicRHI != nullptr);
	GDynamicRHI = nullptr;
}
