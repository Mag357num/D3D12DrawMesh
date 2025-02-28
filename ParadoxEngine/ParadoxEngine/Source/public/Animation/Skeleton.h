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
	vector<FMatrix> JointOffset;

public:
	vector<FJoint>& GetJoints() { return Joints; }
	vector<FPose>& GetBindPoses() { return BindPoses; }
	vector<FMatrix> GetJointOffset(); // Bind Pose inverse matrix: to transform vertex into joint space
	unordered_map<string, int>& GetNameToIndexMap() { return NameToIndexMap; }

	FSkeleton() = default;
	~FSkeleton() = default;
};
