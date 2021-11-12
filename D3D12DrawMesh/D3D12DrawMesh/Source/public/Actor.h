#pragma once
#include "stdafx.h"
#include "ActorComponent.h"

// a object that has its own logic function
class AActor
{
protected:
	vector<shared_ptr<FActorComponent>> Components; // entity of actor

	uint32 DirtyCount = 3; // TODO: change to below
	//uint32 DirtyCount = RHI::GDynamicRHI->GetFrameCount();

public:
	AActor() = default;
	~AActor() = default;

	const bool IsDirty() const { return DirtyCount != 0; }
	void MarkDirty() { DirtyCount = 3; } // TODO: change to below
	//void MarkDirty() { DirtyCount = RHI::GDynamicRHI->GetFrameCount(); }
	void DecreaseDirty() { DirtyCount--; }
};
