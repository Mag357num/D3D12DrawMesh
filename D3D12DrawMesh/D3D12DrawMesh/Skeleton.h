#pragma once
#include "stdafx.h"
#include "Joint.h"

using FPose = FTransform;

class FSkeleton
{
private:
	vector<FJoint> Joints;
	vector<FPose> BindPoses;
	std::unordered_map<string, uint16> NameToIndexMap;

public:
	FSkeleton() = default;
	~FSkeleton() = default;
};
