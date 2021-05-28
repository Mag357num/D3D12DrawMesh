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

class FSkeletalMeshLOD // TODO: only store SkeletalVertexArray and Indice
{
private:
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;
	vector<FSkeletalVertex> SkeletalVertexArray;

	vector<uint32> Indice;

public:
	vector<FStaticVertex>& GetVertice() { return StaticVertexArray; }
	vector<FSkeletalVertex>& GetSkeletalVertice() { return SkeletalVertexArray; }
	vector<FSkinnedWeightVertex>& GetWeightVertice() { return SkinnedWeightVertexArray; }
	vector<uint32>& GetIndice() { return Indice; }
};

class FSkeleton;
class TSkeletalMesh
{
private:
	shared_ptr<FSkeleton> Skeleton;
	vector<FSkeletalMeshLOD> MeshLODs;

public:
	void SetSkeleton(shared_ptr<FSkeleton> Ske) { Skeleton = Ske; };
	void SetSkeletalMeshLods(const vector<FSkeletalMeshLOD>& LODs) { MeshLODs = LODs; }

	vector<FSkeletalMeshLOD>& GetMeshLODs() { return MeshLODs; }

	FSkeleton* GetSkeleton() { return Skeleton.get(); }

	TSkeletalMesh() = default;
	~TSkeletalMesh() = default;
};

class TSkeletalMeshComponent : public FActorComponent
{
private:
	shared_ptr<TSkeletalMesh> SkeletalMesh;
	wstring ShaderFileName;
	FAnimInstance Animator;

public:
	FAnimInstance& GetAnimator() { return Animator; }
	void TickAnimation(const float& ElapsedSeconds) { Animator.UpdateAnimation(ElapsedSeconds); }

	void AddSequence(std::pair<string, shared_ptr<FAnimSequence>> Seq) { Animator.AddSequence(Seq); }
	void InitAnimation() { Animator.initAnimation(this); } // init this when init chararcter

	void SetSkeletalMesh(shared_ptr<TSkeletalMesh> SkeM) { SkeletalMesh = SkeM; }
	void SetShaderFileName(const wstring& Name) { ShaderFileName = Name; }

	wstring& GetShaderFileName() { return ShaderFileName; }
	TSkeletalMesh* GetSkeletalMesh() { return SkeletalMesh.get(); }

	TSkeletalMeshComponent() = default;
	~TSkeletalMeshComponent() = default;
};

class ASkeletalMeshActor : public AActor
{
};