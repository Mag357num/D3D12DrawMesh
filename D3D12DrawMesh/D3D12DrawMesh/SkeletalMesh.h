#pragma once
#include "stdafx.h"
#include "Skeleton.h"
#include "StaticVertex.h"
#include "SkinnedWeightVertex.h"

class FSkeletalMesh
{
private:
	FSkeleton* Skeleton;
	vector<FStaticVertex> StaticVertexArray;
	vector<FSkinnedWeightVertex> SkinnedWeightVertexArray;

public:
	void SetSkeleton(FSkeleton* Ske) { Skeleton = Ske; };

	FSkeletalMesh() = default;
	~FSkeletalMesh() = default;

private:

};

