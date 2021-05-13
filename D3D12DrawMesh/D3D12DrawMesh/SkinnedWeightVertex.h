#pragma once
#include "stdafx.h"

#define MAX_INFLUENCED_JOINT_NUM 4

class FSkinnedWeightVertex
{
private:
	uint16 InfluJointIndice[MAX_INFLUENCED_JOINT_NUM];
	uint8 InfluJointWeights[MAX_INFLUENCED_JOINT_NUM]; // Accuracy is 1/256

public:
	FSkinnedWeightVertex() = default;
	~FSkinnedWeightVertex() = default;
};
