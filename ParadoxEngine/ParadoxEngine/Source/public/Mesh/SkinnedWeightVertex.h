#pragma once
#include "stdafx.h"
#define MAX_INFLUENCED_JOINT_NUM 4

class FSkinnedWeightVertex
{
private:
	vector<uint16> InfluJointIndice;
	vector<uint8> InfluJointWeights; // Accuracy is 1/256

public:
	vector<uint16>& GetJointIndice() { return InfluJointIndice; }
	vector<uint8>& GetJointWeights() { return InfluJointWeights; }

	FSkinnedWeightVertex() = default;
	~FSkinnedWeightVertex() = default;
};
