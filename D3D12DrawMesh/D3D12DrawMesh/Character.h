#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"

class ACharacter : public AActor
{
public:
	void SetSkeletalMeshCom(shared_ptr<FSkeletalMeshComponent> SkeMesh) { Components.push_back(SkeMesh); }
	void Tick(const float& TotalSeconds);

	FSkeletalMeshComponent* GetSkeletalMeshCom() { return Components[0]->As<FSkeletalMeshComponent>(); }

	ACharacter() = default;
	~ACharacter() = default;

private:

};

