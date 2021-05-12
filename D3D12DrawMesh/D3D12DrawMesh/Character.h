#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"

class ACharacter : public AActor
{
private:
	shared_ptr<ASkeletalMeshActor> SkeletalMeshActor;

public:
	void SetSkinnedMesh(shared_ptr<ASkeletalMeshActor> SkeMesh) { SkeletalMeshActor = SkeMesh; }
	void Tick(const float& ElapsedSeconds);

	ACharacter() = default;
	~ACharacter() = default;

private:

};

