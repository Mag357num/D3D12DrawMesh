#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

struct FSkeletalMeshLOD
{
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<uint32> Indices;
};

class FSkeleton;
class FSkeletalMesh
{
private:
	FSkeleton* Skeleton;

	vector<FSkeletalMeshLOD> MeshLODs;

public:
	void SetSkeleton(FSkeleton* Ske) { Skeleton = Ske; };
	void SetSkeletalMeshLods(const vector<FSkeletalMeshLOD>& LODs) { MeshLODs = LODs; }

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
	void TickAnimation(const float& ElapsedSeconds) { Animator.UpdateAnimation(ElapsedSeconds); }
	void InitAnimation(FAnimSequenceBase* Sequence) { Animator.initAnimation(this, Sequence); } // init this when init chararcter

	void SetSkeletalMesh(shared_ptr<FSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }


	FSkeletalMeshComponent() = default;
	~FSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{
};