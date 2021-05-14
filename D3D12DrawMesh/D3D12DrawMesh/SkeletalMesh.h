#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

class FSkeletalMeshLOD
{
private:
	uint16 VertexStride;
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<uint32> Indices;

public:
	void ResizeVertices(uint32 Size) { StaticVertexArray.resize(Size); SkinnedWeightVertexArray.resize(Size); }
	void ResizeIndices(uint32 Size) { Indices.resize(Size); }
	const vector<FStaticVertex>& GetVertices() const { return StaticVertexArray; }
	vector<FSkinnedWeightVertex>& GetWeightVertices() { return SkinnedWeightVertexArray; }
	const vector<uint32>& GetIndices() const { return Indices; }
	const uint32& GetVertexStride() const { return VertexStride; }
	void SetVertexStride(const uint16& Stride) { VertexStride = Stride; }
	void SetVertices(const vector<FStaticVertex>& Param) { StaticVertexArray = Param; }
	void SetWeightVertices(const vector<FSkinnedWeightVertex>& Param) { SkinnedWeightVertexArray = Param; }
	void SetIndices(const vector<uint32>& Param) { Indices = Param; }
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
	void InitAnimation(FAnimSequence* Sequence) { Animator.initAnimation(this, Sequence); } // init this when init chararcter

	void SetSkeletalMesh(shared_ptr<FSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }


	FSkeletalMeshComponent() = default;
	~FSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{
};