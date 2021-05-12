#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "StaticVertex.h"
#include "SkinnedWeightVertex.h"
#include "AnimaInstance.h"
#include "ActorComponent.h"
#include "Actor.h"

class FSkeletalMesh
{
private:
	FSkeleton* Skeleton;
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;

public:
	void SetSkeleton(FSkeleton* Ske) { Skeleton = Ske; };

	FSkeletalMesh() = default;
	~FSkeletalMesh() = default;
};

class FSkeletalMeshComponent : public FActorComponent
{
private:
	FSkeletalMesh* SkeletalMesh;
	FAnimaInstance Animator;

public:
	FSkeletalMeshComponent() = default;
	~FSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{

};