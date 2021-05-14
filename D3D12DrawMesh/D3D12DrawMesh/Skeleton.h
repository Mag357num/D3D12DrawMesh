#pragma once
#include "stdafx.h"
#include "Joint.h"

using FPose = FTransform;

class FSkeleton
{
private:
	vector<FJoint> Joints;
	vector<FPose> BindPoses;
	unordered_map<string, int> NameToIndexMap;

public:
	vector<FJoint>& GetJoints() { return Joints; }
	vector<FPose>& GetBindPoses() { return BindPoses; }
	unordered_map<string, int>& GetNameToIndexMap() { return NameToIndexMap; }

	FSkeleton() = default;
	~FSkeleton() = default;
};
