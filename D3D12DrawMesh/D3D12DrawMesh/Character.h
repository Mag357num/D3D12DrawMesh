#pragma once
#include "stdafx.h"
#include "SkinnedMesh.h"

class FCharacter
{
private:
	FSkinnedMesh* SkinnedMesh;

public:
	void SetSkinnedMesh(FSkinnedMesh* SkiMesh) { SkinnedMesh = SkiMesh; }

	FCharacter() = default;
	~FCharacter() = default;

private:

};

