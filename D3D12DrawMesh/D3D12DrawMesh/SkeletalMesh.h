#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

class FSkeletalMeshLOD
{
private:
	uint32 VertexStride; // TODO: uint16 is enough
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<uint32> Indices;

public:
	vector<FStaticVertex>& GetVertices() { return StaticVertexArray; }
	vector<FSkinnedWeightVertex>& GetWeightVertices() { return SkinnedWeightVertexArray; }
	vector<uint32>& GetIndices() { return Indices; }
	uint32& GetVertexStride() { return VertexStride; }
};

class FSkeleton;
class FSkeletalMesh
{
private:
	shared_ptr<FSkeleton> Skeleton;

	vector<FSkeletalMeshLOD> MeshLODs;

public:
	void SetSkeleton(shared_ptr<FSkeleton> Ske) { Skeleton = Ske; };
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
	void InitAnimation(shared_ptr<FAnimSequence> Sequence) { Animator.initAnimation(this, Sequence); } // init this when init chararcter

	void SetSkeletalMesh(shared_ptr<FSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }


	FSkeletalMeshComponent() = default;
	~FSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{
};