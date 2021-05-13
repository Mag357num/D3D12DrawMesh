#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"

class ACharacter : public AActor
{
private:
	shared_ptr<FSkeletalMeshComponent> SkeletalMeshCom;

public:
	void SetSkeletalMeshCom(shared_ptr<FSkeletalMeshComponent> SkeMesh) { SkeletalMeshCom = SkeMesh; }
	void Tick(const float& ElapsedSeconds);

	FSkeletalMeshComponent* GetSkeletalMeshCom() { return SkeletalMeshCom.get(); }

	ACharacter() = default;
	~ACharacter() = default;

private:

};

