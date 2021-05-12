#pragma once
#include "stdafx.h"

#define MAX_INFLUENCED_JOINT_NUM 4

class FSkinnedWeightVertex
{
private:
	uint16 InfluJointIndex[MAX_INFLUENCED_JOINT_NUM];
	float InfluJointWeight[MAX_INFLUENCED_JOINT_NUM];

public:
	FSkinnedWeightVertex() = default;
	~FSkinnedWeightVertex() = default;

private:

};

