#pragma once
#include "stdafx.h"
#include "Bounding.h"
#include "ActorComponent.h"

enum class EActorType // this enum class shell be same with the ResourceExport plugin in ue4
{
	STATICMESH_ACTOR = 0,
	CAMERA_ACTOR = 1,
	DIRECTIONALLIGHT_ACTOR = 2,
	POINTLIGHT_ACTOR = 3
};

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

	FBox GetComponentsBoundingBox();
};
