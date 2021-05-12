#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"
#include "Actor.h"

class ACharacter : public AActor
{
private:
	FSkeletalMesh* SkinnedMesh;

public:
	void SetSkinnedMesh(FSkeletalMesh* SkiMesh) { SkinnedMesh = SkiMesh; }
	void Tick(const float& ElapsedSeconds);

	ACharacter() = default;
	~ACharacter() = default;

private:

};

