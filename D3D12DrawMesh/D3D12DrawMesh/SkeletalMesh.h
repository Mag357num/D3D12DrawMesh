#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "StaticVertex.h"
#include "SkinnedWeightVertex.h"

class FSkinnedMesh
{
private:
	FSkeleton* Skeleton;
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;

public:
	void SetSkeleton(FSkeleton* Ske) { Skeleton = Ske; };

	FSkinnedMesh() = default;
	~FSkinnedMesh() = default;

private:

};

