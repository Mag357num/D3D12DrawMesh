#pragma once
#include "StaticVertex.h"
#include "AnimInstance.h"
#include "SkinnedWeightVertex.h"
#include "ActorComponent.h"
#include "Actor.h"

struct TSkeletalVertex
{
	FVector Pos;
	FVector Nor;
	FVector2 UV0;
	FVector4 Color;
	array<uint16, 4> InfluJointWeights; // Accuracy is 1/256
	array<uint16, 4> InfluJointIndice;
};

struct TSkeletalMeshLOD
{
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<TSkeletalVertex> SkeletalVertexArray;
	vector<uint32> Indice;
};

class TSkeleton;
class TSkeletalMesh
{
private:
	shared_ptr<TSkeleton> Skeleton;
	vector<TSkeletalMeshLOD> MeshLODs;

public:
	TSkeletalMesh() = default;
	~TSkeletalMesh() = default;

	// skeleton
	void SetSkeleton(shared_ptr<TSkeleton> Ske) { Skeleton = Ske; };
	TSkeleton* GetSkeleton() { return Skeleton.get(); }

	// lod
	void SetSkeletalMeshLods( const vector<TSkeletalMeshLOD>& LODs ) { MeshLODs = LODs; }
	vector<TSkeletalMeshLOD>& GetMeshLODs() { return MeshLODs; }
};

class TSkeletalMeshComponent : public TActorComponent
{
private:
	shared_ptr<TSkeletalMesh> SkeletalMesh;
	FAnimInstance Animator;

public:
	TSkeletalMeshComponent() { Animator.initAnimation(this); };
	~TSkeletalMeshComponent() = default;

	// animator
	FAnimInstance& GetAnimator() { return Animator; }
	void TickAnimation(const float& ElapsedSeconds) { Animator.UpdateAnimation(ElapsedSeconds); }
	void AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq );

	// skeletal mesh
	void SetSkeletalMesh(shared_ptr<TSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }
	TSkeletalMesh* GetSkeletalMesh() { return SkeletalMesh.get(); }
};

class ASkeletalMeshActor : public AActor
{
	void SetSkeletalMeshComponent( shared_ptr<TSkeletalMeshComponent> Com );
	TSkeletalMeshComponent* GetSkeletalMeshComponent();
};