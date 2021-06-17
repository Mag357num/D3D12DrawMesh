#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

struct FSkeletalVertex
{
	FVector Pos;
	FVector Nor;
	FVector2 UV0;
	FVector4 Color;
	array<uint16, 4> InfluJointWeights; // Accuracy is 1/256
	array<uint16, 4> InfluJointIndice;
};

struct FSkeletalMeshLOD
{
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<FSkeletalVertex> SkeletalVertexArray;
	vector<uint32> Indice;
};

class FSkeleton;
class FSkeletalMesh
{
private:
	shared_ptr<FSkeleton> Skeleton;
	vector<FSkeletalMeshLOD> MeshLODs;

public:
	FSkeletalMesh() = default;
	~FSkeletalMesh() = default;

	// skeleton
	void SetSkeleton(shared_ptr<FSkeleton> Ske) { Skeleton = Ske; };
	FSkeleton* GetSkeleton() { return Skeleton.get(); }

	// lod
	void SetSkeletalMeshLods( const vector<FSkeletalMeshLOD>& LODs ) { MeshLODs = LODs; }
	vector<FSkeletalMeshLOD>& GetMeshLODs() { return MeshLODs; }
};

class FSkeletalMeshComponent : public FMeshComponent
{
private:
	shared_ptr<FSkeletalMesh> SkeletalMesh;
	FAnimInstance Animator;

public:
	FSkeletalMeshComponent() { Animator.initAnimation(this); };
	~FSkeletalMeshComponent() = default;

	// animator
	FAnimInstance& GetAnimator() { return Animator; }
	void TickAnimation(const float& ElapsedSeconds) { Animator.TickAnimation(ElapsedSeconds); }
	void AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq );

	// skeletal mesh
	void SetSkeletalMesh(shared_ptr<FSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }
	FSkeletalMesh* GetSkeletalMesh() { return SkeletalMesh.get(); }
};

class ASkeletalMeshActor : public AActor
{
	void SetSkeletalMeshComponent( shared_ptr<FSkeletalMeshComponent> Com );
	FSkeletalMeshComponent* GetSkeletalMeshComponent();
};