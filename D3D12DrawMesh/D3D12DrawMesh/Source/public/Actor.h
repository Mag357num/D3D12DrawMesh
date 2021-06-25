#pragma once
#include "stdafx.h"
#include "Bounding.h"
#include "ActorComponent.h"

enum class EActorType // this enum class shell be same with the EActorType in ue4 ResourceExport plugin 
{
	STATICMESH_ACTOR = 0,
	CAMERA_ACTOR = 1,
	DIRECTIONALLIGHT_ACTOR = 2,
	POINTLIGHT_ACTOR = 3,
	SKELETALMESH_ACTOR = 4,
	UNKNOW = 5
};

// a object that has its own logic function
class AActor
{
protected:
	string ActorName;
	shared_ptr<FSceneComponent> RootComponent;
	vector<shared_ptr<FSceneComponent>> OwnedComponents; // store component here

	uint32 DirtyCount = 3; // TODO: change to below
	//uint32 DirtyCount = RHI::GDynamicRHI->GetFrameCount();

public:
	AActor() = default;
	virtual ~AActor() = default;

	const bool IsDirty() const { return DirtyCount != 0; }
	void MarkDirty() { DirtyCount = 3; } // TODO: change to below
	//void MarkDirty() { DirtyCount = RHI::GDynamicRHI->GetFrameCount(); }
	void DecreaseDirty() { DirtyCount--; }

	void SetName(string Name) { ActorName = Name; }
	FBox GetComponentsBoundingBox();
	FSceneComponent* GetRootComponent() { return RootComponent.get(); }
	vector<shared_ptr<FSceneComponent>>& GetOwnedComponents() { return OwnedComponents; }

	void AddOwnedComponent(shared_ptr<FSceneComponent> Com) { OwnedComponents.push_back(Com); Com->SetOwner(this); }
};
