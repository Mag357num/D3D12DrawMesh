#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

class FSkeleton;
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
	shared_ptr<FSkeletalMesh> SkeletalMesh;
	FAnimInstance Animator;

public:
	FAnimInstance GetAnimator() { return Animator; }
	void TickAnimation() { Animator.UpdateAnimation(); }

	FSkeletalMeshComponent() = default;
	FSkeletalMeshComponent(FAnimSequenceBase* Sequence) { Animator.initAnimation(this, Sequence); }
	~FSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{
};