#pragma once
#include "stdafx.h"
#include "SkeletalMesh.h"

class FCharacter
{
private:
	FSkeletalMesh* SkinnedMesh;

public:
	void SetSkinnedMesh(FSkeletalMesh* SkiMesh) { SkinnedMesh = SkiMesh; }

	FCharacter() = default;
	~FCharacter() = default;

private:

};

